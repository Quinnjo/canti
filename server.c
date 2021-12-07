#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <dirent.h>

#include "game.h"
#include "list.h"
#include "board.h"
#include "command.h"

#define MAX_CONNECTIONS 16
#define MAX_GAMES 8
#define MAX_SPECTATORS 8

#define TIMEOUT_S 0
#define TIMEOUT_US 10000 /* 10 ms, 10000 us */

/* global integer */
int debug;

typedef struct Game {
  Position* pos;
  int status; /* WAITING, ONGOING, COMPLETED */
  int id;
  int n_players;
  int n_spectators;
  int players[2]; /* player file descriptors */
  int white; /* white file descriptor */
  int black; /* black file descriptor */
  int spectators[MAX_SPECTATORS]; /* spectator file descriptors */
  pthread_mutex_t mtx; /* mutex */
  pthread_cond_t ready; /* ready to start signal */
} Game;

/*
 * initialize with extra space for the array
 */
typedef struct ProtectedIntArray {
  pthread_mutex_t mtx;
  int arr[0];
} ProtectedIntArray;

typedef struct ProtectedGameArray {
  pthread_mutex_t mtx;
  Game* arr[0];
} ProtectedGameArray;

void checkError(int status,int line) {
  if (status < 0) {
    printf("socket error(%d)-%d: [%s]\n",getpid(),line,strerror(errno));
    exit(-1);
  }
}


int logStr(char* str) {
  if(debug) {
    printf("%s\n", str);
  }
}


#define WAITING 2
#define ONGOING 1
#define COMPLETED 0
/* creates a server-side socket that binds to the given port number and listens for TCP connect requests */
int bindAndListen(int port)
{
  int sid = socket(PF_INET,SOCK_STREAM,0);
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  int status = bind(sid,(struct sockaddr*)&addr,sizeof(addr));
  checkError(status,__LINE__);
  status = listen(sid,10);
  checkError(status,__LINE__);
  return sid;
}

typedef struct HubThreadArgs {
  ProtectedIntArray* clients;
  ProtectedGameArray* games;
} HubThreadArgs;

typedef struct CommandHubThreadArgs {
  int fd;
  ProtectedIntArray* clients;
  ProtectedGameArray* games;
} CommandHubThreadArgs;

Game* newGame(int id) {
  Game* game = malloc(sizeof(Game));
  game->pos = newPosition();
  game->status = WAITING;
  game->id = id;
  game->n_players = 0;
  game->n_spectators = 0;
  game->white = game->black = 0;
  pthread_mutex_init(&game->mtx, NULL);
  pthread_cond_init(&game->ready, NULL);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    game->spectators[i] = 0;
  }
  return game;
}

void destroyGame(Game* game) {
  freePosition(game->pos);
  free(game->pos);
  pthread_mutex_destroy(&game->mtx);
  pthread_cond_destroy(&game->ready);
  free(game);
}

/* replace first 0 in arr with n */
/* return 1 on success, 0 otherwise */
int addInt(int n, int arr[]) {
  int sz = sizeof(arr) / sizeof(*arr);
  for(int i = 0; i < sz; i++) {
    if(arr[i] == 0) {
      arr[i] = n;
      return 1;
    }
  }
  return 0;
}

/* replace n in arr with 0 */
/* return 1 on success, 0 otherwise */
int removeInt(int n, int arr[]) {
  int sz = sizeof(arr) / sizeof(*arr);
  for(int i = 0; i < sz; i++) {
    if(arr[i] == n) {
      arr[i] = 0;
      return 1;
    }
  }
  return 0;
}

/* returns 1 if there is data to read in the file descriptor fd, 0 otherwise */
int dataToRead(int fd) {
  fd_set fdin;
  FD_ZERO(&fdin);
  FD_SET(fd, &fdin);
  struct timeval timeout = {0, 0}; /* ensure that select doesn't block */
  int selectResult = select(fd+1, &fdin, NULL, NULL, &timeout);
  return selectResult > 0;
}

/* sends the current board to players and spectators */
/* Potential issue: does boardStr function null terminate the string? */
void sendBoard(Game* game) {
  char wbuf[BOARD_STRLEN], bbuf[BOARD_STRLEN];
  boardToBufWhite(game->pos->board, wbuf);
  boardToBufBlack(game->pos->board, bbuf);
  write(game->white, wbuf, BOARD_STRLEN);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      write(game->spectators[i], wbuf, BOARD_STRLEN);
    }
  }
  write(game->black, bbuf, BOARD_STRLEN);
}


#define COMMAND_BUFSIZE 16
#define COMMANDARG_BUFSIZE 16

#define CHECKMATE 0
#define RESIGNATION 1
#define STALEMATE 0

void deconstructGame(Game* game) {
  game->status = COMPLETED;

  /* close the connections */
  close(game->white);
  close(game->black);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      close(game->spectators[i]);
    }
  }

  pthread_mutex_unlock(&game->mtx);
  pthread_exit(NULL); /* kill the game thread */
}

/* Use to end the game when a player disconnects */
/* fd is the socket id of the player that disconnected */
void endGameDisconnect(Game* game, int fd) {

  /* announce that the game is over */
  char msg[] = "Uh oh. Someone disconnected! The game is now over.\n";
  if(game->white != fd) {
    /* white didn't DC */
    write(game->white, msg, sizeof(msg));
  }
  if(game->black != fd) {
    /* black didn't DC */
    write(game->black, msg, sizeof(msg));
  }
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      write(game->spectators[i], msg, sizeof(msg));
    }
  }

  /* deconstruct the game */
  deconstructGame(game);
}

void endGameWhite(Game* game, int reason) {
  char buf[32];
  int wb;
  if(reason == CHECKMATE) {
    wb = sprintf(buf, "White wins by %s!\n", "checkmate");
  } else if(reason == RESIGNATION) {
    wb = sprintf(buf, "White wins by %s.\n", "resignation");
  } else {
    wb = sprintf(buf, "White wins.\n");
  }

  /* announce that the game is over */
  write(game->white, buf, wb+1);
  write(game->black, buf, wb+1);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      write(game->spectators[i], buf, wb+1);
    }
  }

  /* deconstruct and end the game */
  deconstructGame(game);
}

void endGameBlack(Game* game, int reason) {
  char buf[32];
  int wb;
  if(reason == CHECKMATE) {
    wb = sprintf(buf, "Black wins by %s!\n", "checkmate");
  } else if(reason == RESIGNATION) {
    wb = sprintf(buf, "Black wins by %s.\n", "resignation");
  } else {
    wb = sprintf(buf, "Black wins.\n");
  }

  /* announce */
  write(game->white, buf, wb+1);
  write(game->black, buf, wb+1);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      write(game->spectators[i], buf, wb+1);
    }
  }

  /* deconstruct */
  deconstructGame(game);
}

void endGameDraw(Game* game, int reason) {
  char buf[32];
  int wb;
  if(reason == STALEMATE) {
    wb = sprintf(buf, "The game is drawn by %s.\n", "stalemate");
  } else {
    wb = sprintf(buf, "The game is drawn.\n");
  }

  /* announce */
  write(game->white, buf, wb+1);
  write(game->black, buf, wb+1);
  for(int i = 0; i < MAX_SPECTATORS; i++) {
    if(game->spectators[i]) {
      write(game->spectators[i], buf, wb+1);
    }
  }

  /* deconstruct */
  deconstructGame(game);
}

void checkEndGame(Game* game) {
  int check = inCheck(game->pos);
  int n = numberLegalMoves(game->pos);
  if(game->pos->toMove == WHITE) {
    if(n == 0) {
      if(check == WHITE) {
	/* white is checkmated, black wins */
	endGameBlack(game, CHECKMATE);
      } else {
	/* white is stalemated, draw */
	endGameDraw(game, STALEMATE);
      }
    }
  } else if(game->pos->toMove == BLACK) {
    if(n == 0) {
      if(check == BLACK) {
	/* black is checkmated */
	endGameWhite(game, CHECKMATE);
      } else {
	/* black is stalemated */
	endGameDraw(game, STALEMATE);
      }
    }
  }
}

/* command functions for users in a game */

void commandMove(int fd, Game* game) {
  /* get Move object from command input */
  logStr("processing move input");
  char buf[5];
  Move m;
  if(dataToRead(fd)) {
    int rb = read(fd, buf, 4);
    buf[5] = 0;
    if(rb < 4) {
      /* there must be four characters in the input */
      char msg[] = "Could not process the given move.\n";
      write(fd, msg, sizeof(msg));
      return;
    }
    char s1[3];
    s1[0] = buf[0], s1[1] = buf[1], s1[2] = 0;
    char s2[3];
    s2[0] = buf[2], s2[1] = buf[3], s2[2] = 0;
    int sq1 = squareFromString(s1);
    int sq2 = squareFromString(s2);
    if(sq1 == -1 || sq2 == -1) {
      /* at least one of the squares is invalid */
      char msg[] = "Invalid move.\n";
      write(fd, msg, sizeof(msg));
      return;
    }
    m.start = sq1;
    m.end = sq2;
  } else {
    /* there is no data to read from this socket */
    char msg[] = "Please include the move (e.g. e2e4 or g8f6).\n";
    write(fd, msg, sizeof(msg));
    return;
  }

  if(game->pos->toMove == WHITE) {
    if(fd == game->white) {
      /* white is entering their move */
      if(moveIsLegal(&m, game->pos)) {
	applyMoveToPosition(&m, game->pos);
      } else {
	/* the move is not legal */
	char msg[] = "Illegal move.\n";
	write(fd, msg, sizeof(msg));
	return;
      }
    } else {
      /* black is moving out of turn */
      char msg[] = "It is not your turn.\n";
      write(fd, msg, sizeof(msg));
    }
  } else if(game->pos->toMove == BLACK) {
    if(fd == game->black) {
      /* black is entering their move */
      if(moveIsLegal(&m, game->pos)) {
	applyMoveToPosition(&m, game->pos);
      } else {
	/* move is illegal */
	char msg[] = "Illegal move.\n";
	write(fd, msg, sizeof(msg));
	return;
      }
    } else {
      /* white is moving out of turn */
      char msg[] = "It is not your turn.\n";
      write(fd, msg, sizeof(msg));
      return;
    }
  } else {
    /* something is wrong with the game data */
    char msg[] = "Something went wrong.\n";
    write(fd, msg, sizeof(msg));
    return;
  }

  /* TODO: see if boardStr functions null terminate the strings */
  sendBoard(game);

  /* check for game end conditions */
  checkEndGame(game);
}

void commandListMoves(int fd, Game* game) {
  char msg[] = "List of legal moves:\n";
  write(fd, msg, sizeof(msg));
  LList* moves = genLegalMoves(game->pos);
  char sq1[2], sq2[2];
  for(int i = 0; i < moves->len; i++) {
    /* retrieve the move and get the string from that move */
    Move* m = itemAtIndex(moves, i);
    squareToString(sq1, m->start);
    squareToString(sq2, m->end);
    /* write each square, then make a new line */
    write(fd, sq1, 2);
    write(fd, sq2, 2);
    write(fd, (void*)"\n", 1);
  }
  write(fd, (void*)0, 1); /* terminate the sent string */
  freeList(moves); /* free the generated list */
  free(moves);
}

void commandMessage(int fd, Game* game) {
  int recipient = (fd == game->white) ? game->black : game->white;
  char sender = (fd == game->white) ? 'W' : 'B';

  char buf[512];
  if(dataToRead(fd)) {
    /* read the messsage from the client into buf */
    char msg[512];
    for(int i = 0; i < sizeof(buf); i++) {
      int rb = read(fd, buf+i, 1);
      if(rb == 0) {
	/* EOF or closed connection */
	/* this client is gone */
	/* end the game */
	endGameDisconnect(game, fd);
      }
      /* check for newline */
      if(buf[i] == '\n') {
	buf[i] = 0;
	break;
      }
    }
    buf[511] = 0;

    /* relay the message to everyone */
    int wb = snprintf(msg, sizeof(msg), "%c says: %s\n", sender, buf);
    write(recipient, msg, wb); /* send to the recipient player */
    /* send to the spectators */
    for(int i = 0; i < MAX_SPECTATORS; i++) {
      if(game->spectators[i]) {
	write(game->spectators[i], msg, wb);
      }
    }
  } else {
    /* no message was provided */
    /* do nothing ! */
    /* potential bug: what if the client EOFs after a message command? */
  }
}

/* NOT IMPLEMENTED */
/*
void commandDraw(int fd, Game* game) {
  return;
}
*/

void commandResign(int fd, Game* game) {
  if(fd == game->white) {
    /* white resigned */
    endGameBlack(game, RESIGNATION);
  } else {
    /* black resigned */
    endGameWhite(game, RESIGNATION);
  }
}

void processCommandPlayer(int fd, char c[], Game* game) {
  if(strcmp(c, "move") == 0) {
    /* move command */
    commandMove(fd, game);
  } else if(strcmp(c, "listmoves") == 0) {
    /* list moves command */
    commandListMoves(fd, game);
  } else if(strcmp(c, "message") == 0) {
    /* send a message command */
    commandMessage(fd, game);
  } else if(strcmp(c, "resign") == 0) {
    /* resign command */
    commandResign(fd, game);
  } else {
    /* unrecognized command */
    /*
    char msg[] = "Your command was not recognized.\n";
    write(fd, msg, sizeof(msg));
    */
  }
}

void handleCommandPlayer(int fd, Game* game) {
  char buf[COMMAND_BUFSIZE];
  for(int i = 0; i < COMMAND_BUFSIZE; i++) {
    int rb = read(fd, buf+i, 1); /* read one char into buf */

    /* the connection is closed or there is an EOF */
    if(rb == 0) {
      /* this player resigns by disconnecting */
      game->n_spectators -= removeInt(fd, game->spectators);
      close(fd);
      return;
    }

    /* stop reading if we hit a space or newline or EOF*/
    if(buf[i] == ' ' || buf[i] == '\n' || buf[i] == EOF) {
      buf[i] = 0;
      break;
    }
  }
  /* buf must always end in a 0 */
  buf[COMMAND_BUFSIZE-1] = 0;
  processCommandPlayer(fd, buf, game);
  logStr("finished processing player command");
}

/* spectator commands */
void commandDisconnectSpectator(int fd, Game* game) {
  game->n_spectators -= removeInt(fd, game->spectators);
  char msg[] = "You have been successfully disconnected.\n"; 
  write(fd, msg, sizeof(msg));
  close(fd);
}

void processCommandSpectator(int fd, char c[], Game* game) {
  if(strcmp(c, "disconnect") == 0) {
    /* disconnect this client from spectating */
    commandDisconnectSpectator(fd, game);
  } else {
    /* unrecognized command */
    char msg[] = "Your command was not recognized.\n";
    write(fd, msg, sizeof(msg));
  }
}

void handleCommandSpectator(int fd, Game* game) {
  char buf[COMMAND_BUFSIZE];
  for(int i = 0; i < COMMAND_BUFSIZE; i++) {
    int rb = read(fd, buf+i, 1); /* read one char into buf */

    /* the connection is closed or there is an EOF */
    if(rb == 0) {
      game->n_spectators -= removeInt(fd, game->spectators);
      return;
    }

    /* stop reading if we hit a space or newline or EOF*/
    if(buf[i] == ' ' || buf[i] == '\n' || buf[i] == EOF) {
      buf[i] = 0;
      break;
    }
  }
  /* buf must always end in a 0 */
  buf[COMMAND_BUFSIZE-1] = 0;
  processCommandSpectator(fd, buf, game);
}

/* receives a game pointer */
void* gameHandler(void* data) {
  logStr("new game handler thread");
  pthread_detach(pthread_self());
  Game* game = data;
  pthread_mutex_lock(&game->mtx);

  /* wait for there to be 2 players */
  while(game->n_players < 2 && game->status != ONGOING) {
    pthread_cond_wait(&game->ready, &game->mtx);
  }

  char wmsg[] = "The game has started. You have the white pieces.\n";
  char bmsg[] = "The game has started. You have the black pieces.\n";
    /* randomize colors */
  if(random() % 2 == 0) {
    game->white = game->players[0];
    game->black = game->players[1];
  } else {
    game->white = game->players[1];
    game->black = game->players[0];
  }

  sendBoard(game);
  write(game->white, wmsg, sizeof(wmsg));
  write(game->black, bmsg, sizeof(bmsg));

  pthread_mutex_unlock(&game->mtx);
    
  while(1) {
    /* there are now 2 players and we are ready to accept input and start the game */
    fd_set fdin;
    FD_ZERO(&fdin);
    /* add the players and any spectators to the set */
    pthread_mutex_lock(&game->mtx);
    FD_SET(game->players[0], &fdin);
    FD_SET(game->players[1], &fdin);
    int max_fd = (game->players[0] > game->players[1]) ? game->players[0] : game->players[1];

    for(int i = 0; i < MAX_SPECTATORS; i++) {
      if(game->spectators[i]) {
	FD_SET(game->spectators[i], &fdin);
	max_fd = (game->spectators[i] > max_fd) ? game->spectators[i] : max_fd;
      }
    }
    pthread_mutex_unlock(&game->mtx);

    struct timeval timeout = {TIMEOUT_S, TIMEOUT_US};
    int selectResult = select(max_fd+1, &fdin, NULL, NULL, &timeout);

    if(selectResult > 0) {
      /* a client has new input for us to process */
      pthread_mutex_lock(&game->mtx);
      /* was the input from one of the players? */
      if(FD_ISSET(game->white, &fdin)) {
	handleCommandPlayer(game->white, game);
      } else if(FD_ISSET(game->black, &fdin)) {
	handleCommandPlayer(game->black, game);
      }

      for(int i = 0; i < MAX_SPECTATORS; i++) {
	int cur_fd = game->spectators[i];
	if(FD_ISSET(cur_fd, &fdin)) {
	  /* Handle spectator command*/
	  handleCommandSpectator(cur_fd, game);
	}
      }
      pthread_mutex_unlock(&game->mtx);
    }
  }
  return NULL;
}

/* Command functions for users in the hub room */

/* newgame */
/* Searches for an empty spot or a game that is already completed */
/* Starts a new thread to handle the game */
void commandNewGame(int fd, ProtectedIntArray* clients, ProtectedGameArray* games) {
  pthread_mutex_lock(&games->mtx);
  int success = 0;
  for(int i = 0; i < MAX_GAMES; i++) {
    /* is this an empty spot? */
    if(games->arr[i] == NULL) {
      logStr("new game in empty slot");
      /* create the new game and give it our client */
      games->arr[i] = newGame(i);
      games->arr[i]->players[0] = fd;
      games->arr[i]->n_players = 1;
      removeInt(fd, clients->arr); /* remove fd from the client list */

      /* start the game thread */
      pthread_t pid;
      pthread_create(&pid, NULL, gameHandler, (void*)games->arr[i]);
      success = 1;
      break;
    }

    /* is this spot held by a game that has been completed? */
    /* if so, destroy this game and replace it */
    pthread_mutex_lock(&games->arr[i]->mtx);
    if(games->arr[i]->status == COMPLETED) {
      logStr("cleaning up dead game");
      pthread_mutex_unlock(&games->arr[i]->mtx);
      /* the game thread should have already died, so no one should grab this mutex */
      destroyGame(games->arr[i]);

      /* create the new game and give it our client */
      games->arr[i] = newGame(i);
      games->arr[i]->players[0] = fd;
      games->arr[i]->n_players = 1;
      removeInt(fd, clients->arr); /* remove fd from the client list */

      /* start the game thread */
      pthread_t pid;
      pthread_create(&pid, NULL, gameHandler, (void*)games->arr[i]);
      success = 1;
      break;
    }
    pthread_mutex_unlock(&games->arr[i]->mtx);
  }
  pthread_mutex_unlock(&games->mtx);
  if(!success) {
    /* we did not successfully add a game */
    char msg[] = "There are too many ongoing games to start a new game.\n";
    write(fd, msg, sizeof(msg));
  } else {
    /* we successfully added a game */
    char msg[] = "Created a new game. Please wait for another player to join.\n";
    write(fd, msg, sizeof(msg));
  }
}

void commandJoinPlay(int fd, ProtectedIntArray* clients, ProtectedGameArray* games) {
  char buf[16];
  int id;

  /* can probably reduce the size of the critical section here */
  pthread_mutex_lock(&games->mtx);
  /* we must make sure that there is data to read in the file descriptor */
  if(dataToRead(fd)) {
    for(int i = 0; i < sizeof(buf); i++) {
      int rb = read(fd, buf+i, 1);
      if(rb == 0) {
	/* end of file or the connection has closed */
	/* either way, disconnect this client and do nothing */
	removeInt(fd, clients->arr);
	close(fd);
	return;
      }
      /* we've hit a newline */
      if(buf[i] == '\n') {
	buf[i] = 0;
	break;
      }
    }
    buf[15] = 0;
    id = atoi(buf);
  } else {
    /* no game id was provided */
    /* pick the first game that is available */
    for(int i = 0; i < MAX_GAMES; i++) {
      if(games->arr[i]) {
	pthread_mutex_lock(&games->arr[i]->mtx);
	if(games->arr[i]->status == WAITING) {
	  id = i;
	  pthread_mutex_unlock(&games->arr[i]->mtx);
	  break;
	}
	pthread_mutex_unlock(&games->arr[i]->mtx);
      }
    }
  }

  if(id >= 0 && id < MAX_GAMES) {
    if(games->arr[id]) {
      pthread_mutex_lock(&games->arr[id]->mtx);
      if(games->arr[id]->status == WAITING) {
	/* add this fd to this game */
	/* the game must already have one player */
	games->arr[id]->players[1] = fd;
	games->arr[id]->n_players = 2;
	games->arr[id]->status = ONGOING;
	removeInt(fd, clients->arr);
	pthread_cond_signal(&games->arr[id]->ready); /* signal that the game is ready to start */
      } else {
	/* this game doesn't need a player */
	char msg[] = "This game doesn't need a player. Did you mean to join as a spectator?\n";
	write(fd, msg, sizeof(msg));
      }
      pthread_mutex_unlock(&games->arr[id]->mtx);
    } else {
      /* game with that id doesn't exist */
      char msg[] = "There is no game with that number.\n";
      write(fd, msg, sizeof(msg));
    }
  } else {
    /* the id is invalid */
    char msg[] = "An error occurred processing the given number.\n";
    write(fd, msg, sizeof(msg));
  }
  pthread_mutex_unlock(&games->mtx);
}

void commandJoinSpec(int fd, ProtectedIntArray* clients, ProtectedGameArray* games) {
  char buf[16];
  int id;
  pthread_mutex_lock(&games->mtx);
  /* we must make sure that there is data to read in the file descriptor */
  if(dataToRead(fd)) {
    for(int i = 0; i < sizeof(buf); i++) {
      int rb = read(fd, buf+i, 1);
      if(rb == 0) {
	/* end of file or the connection has closed */
	/* either way, disconnect this client and do nothing */
	removeInt(fd, clients->arr);
	return;
      }
      /* we've hit a newline */
      if(buf[i] == '\n') {
	buf[i] = 0;
	break;
      }
    }
    buf[15] = 0;
    id = atoi(buf);
  } else {
    /* no id was provided */
    char msg[] = "Please provide the ID of the game you wish to spectate.\n";
    write(fd, msg, sizeof(msg));
    /* return here? unlock mutex? */
    pthread_mutex_unlock(&games->mtx);
    return;
  }

  if(id >= 0 && id < MAX_GAMES) {
    if(games->arr[id]) {
      pthread_mutex_lock(&games->arr[id]->mtx);
      if(games->arr[id]->status == COMPLETED) {
	/* this game is already over */
	char msg[] = "Sorry, but this game has already finished.\n";
	write(fd, msg, sizeof(msg));
      } else if(games->arr[id]->n_spectators < MAX_SPECTATORS) {
	/* there is room to spectate */
	addInt(fd, games->arr[id]->spectators); /* add this fd to the spectator list */
	games->arr[id]->n_spectators += 1;
	removeInt(fd, clients->arr); /* remove this fd from the hub room */
      } else {
	/* there isn't room to spectate */
	char msg[] = "There are already too many users spectating this game.\n";
	write(fd, msg, sizeof(msg));
      }
      pthread_mutex_unlock(&games->arr[id]->mtx);
    } else {
      /* a game with that id does not exist */
      char msg[] = "There is no game with that number.\n";
      write(fd, msg, sizeof(msg));
    }
  } else {
    /* the ID is invalid */
    char msg[] = "An error occurred processing the given number.\n";
    write(fd, msg, sizeof(msg));
  }
  pthread_mutex_unlock(&games->mtx);
}

void commandListGames(int fd, ProtectedIntArray* clients, ProtectedGameArray* games) {
  char intro[] = "List of current games (ID: players/total, spectators/total):\n";
  write(fd, intro, sizeof(intro));
  char buf[32];
  pthread_mutex_lock(&games->mtx);
  for(int i = 0; i < MAX_GAMES; i++) {
    if(games->arr[i]) {
      pthread_mutex_lock(&games->arr[i]->mtx);
      sprintf(buf, "%d: %d/2, %d/%d\n", i, games->arr[i]->n_players, games->arr[i]->n_spectators, MAX_SPECTATORS);
      pthread_mutex_unlock(&games->arr[i]->mtx);
      write(fd, buf, strlen(buf)+1);
    }
  }
  pthread_mutex_unlock(&games->mtx);
}

/* disconnect a client from the server */
void commandDisconnect(int fd, ProtectedIntArray* clients, ProtectedGameArray* games) {
  removeInt(fd, clients->arr);
  char msg[] = "You have been successfully disconnected.\n";
  write(fd, msg, sizeof(msg));
  close(fd);
}


/* given a command c sent to the server from a client in the hub room */
void processCommandHub(int fd, char c[], ProtectedIntArray* clients, ProtectedGameArray* games) {
  if(strcmp(c, "newgame") == 0) {
    /* start a new game */
    commandNewGame(fd, clients, games);
  } else if(strcmp(c, "joinplay") == 0) {
    /* join a game as a player */
    commandJoinPlay(fd, clients, games);
  } else if(strcmp(c, "joinspec") == 0) {
    /* join a game as a spectator */
    commandJoinSpec(fd, clients, games);
  } else if(strcmp(c, "listgames") == 0) {
    /* list ongoing games */
    commandListGames(fd, clients, games);
  } else if(strcmp(c, "disconnect") == 0) {
    /* disconnect */
    commandDisconnect(fd, clients, games);
  } else {
    char msg[] = "Your command was not recognized.\n";
    write(fd, msg, sizeof(msg));
    logStr("client sent unrecognized command");
    printf("unrecognized: %s\n", c);
    /* unrecognized command */
  }
}

/* called by the hub room thread to handle input from the clients */
void* handleCommandHub(void* data) {
  /* unpack arguments */
  CommandHubThreadArgs* args = data;
  int fd = args->fd;
  ProtectedIntArray* clients = args->clients;
  ProtectedGameArray* games = args->games;
  /* detach this thread, if it is a thread */
  //pthread_detach(pthread_self());
  /* Extract the command */
  char buf[COMMAND_BUFSIZE];
  for(int i = 0; i < COMMAND_BUFSIZE; i++) {
    int rb = read(fd, buf+i, 1); /* read one char into buf */

    /* the connection is closed or there is an EOF */
    if(rb == 0) {
      removeInt(fd, clients->arr);
      return NULL;
    }

    /* stop reading if we hit a space or newline or EOF*/
    if(buf[i] == ' ' || buf[i] == '\n' || buf[i] == EOF) {
      buf[i] = 0;
      break;
    }
  }
  /* buf must always end in a 0 */
  buf[COMMAND_BUFSIZE-1] = 0;
  processCommandHub(fd, buf, clients, games);

  return NULL;
}

void* hubRoom(void* data) {
  HubThreadArgs* args = data;
  ProtectedIntArray* clients = args->clients;
  ProtectedGameArray* games = args->games;

  logStr("Hub thread ready");

  /* the main thread adds new connections to the client list, we must listen to them */
  while(1) {
    fd_set fdin;
    FD_ZERO(&fdin);
    int max_fd = 0;
    /* add the active clients to the fd set */
    pthread_mutex_lock(&clients->mtx);
    for(int i = 0; i < MAX_CONNECTIONS; i++) {
      int cur_fd = clients->arr[i];
      if(cur_fd) {
	/* this is an active client file descriptor */
	FD_SET(cur_fd, &fdin);
	max_fd = (cur_fd > max_fd) ? cur_fd : max_fd;
      }
    }
    pthread_mutex_unlock(&clients->mtx);

    /* select */
    struct timeval timeout = {TIMEOUT_S, TIMEOUT_US}; /* wait 10 milliseconds for input */
    int selectResult = select(max_fd+1, &fdin, NULL, NULL, &timeout);

    if(selectResult > 0) {
      logStr("New input from a client");
      /* TODO: watch out for deadlocking during comand execution */
      /* potential solution: count up to max_fd and don't use mutex */
      pthread_mutex_lock(&clients->mtx);
      for(int i = 0; i < MAX_CONNECTIONS; i++) {
	int cur_fd = clients->arr[i];
	if(FD_ISSET(cur_fd, &fdin)) {
	  /* client cur_fd has new input to process */
	  logStr("Found the ready client");
	  /* handle the client's command */
	  /* perhaps make a new thread to do this? */
	  CommandHubThreadArgs args = {cur_fd, clients, games};
	  /*
	  pthread_t pid;
	  pthread_create(&pid, NULL, handleCommandHub, &args);
	  */

	  /* right now, every command executed has the mutex for the client list already */
	  logStr("handling command");
	  handleCommandHub(&args);
	  break;
	}
      }
      pthread_mutex_unlock(&clients->mtx);
    }
  }
  return NULL;
}

int main(int argc, char* argv[]) {
  /* set up logging */
  if(argc > 1) {
    /* debug mode on */
    debug = 1;
    logStr("debug mode on!");
  } else {
    debug = 0;
  }

  /* create thread-safe arrays in which to keep the clients and games */
  ProtectedIntArray* clients = malloc(sizeof(ProtectedIntArray) + sizeof(int)*MAX_CONNECTIONS);
  ProtectedGameArray* games = malloc(sizeof(ProtectedGameArray) + sizeof(Game)*MAX_GAMES);
  for(int i = 0; i < MAX_CONNECTIONS; i++) {
    clients->arr[i] = 0;
  }
  for(int i = 0; i < MAX_GAMES; i++) {
    games->arr[i] = NULL;
  }
  pthread_mutex_init(&clients->mtx, NULL);
  pthread_mutex_init(&games->mtx, NULL);

  /* create the hub room thread */
  printf("Creating Hub Room thread\n");
  HubThreadArgs args = {clients, games};
  pthread_t hub;
  pthread_create(&hub, NULL, hubRoom, &args);
  int sid = bindAndListen(PORT_NUMBER);

  char buf[16];
  while(1) {

    fd_set fdin;
    FD_ZERO(&fdin);
    FD_SET(0, &fdin);
    FD_SET(sid, &fdin);

    int selectResult = select(sid+1, &fdin, NULL, NULL, NULL);

    if(selectResult > 0) {
      if(FD_ISSET(0, &fdin)) {
	int rb = read(0, buf, 16);
	if(strncmp("!exit", buf, 5) == 0) {
	  /* exit the server */
	  printf("Received kill command, terminating all threads...\n");
	  close(sid);
	  exit(0);
	}
      } else if(FD_ISSET(sid, &fdin)) {
	/* a new connection is incoming */
	struct sockaddr_in cli;
	socklen_t cliSize = (socklen_t)sizeof(struct sockaddr_in);
	int fd = accept(sid, (struct sockaddr*)&cli, &cliSize);
	/* we've accepted a new connection, find a slot for it in the clients array */
	int accepted = 0;
	pthread_mutex_lock(&clients->mtx);
	for(int i = 0; i < MAX_CONNECTIONS; i++) {
	  if(clients->arr[i] == 0) {
	    clients->arr[i] = fd;
	    printf("Accepted a new client with descriptor %d in slot %d\n", fd, i);
	    char msg[] = "Connection accepted. Enter commands as you wish. Type \"help\" for help.\n";
	    write(fd, msg, sizeof(msg));
	    accepted = 1;
	    break;
	  }
	}
	if(!accepted) {
	  printf("Couldn't fit a client in with descriptor %d. Closing socket...\n", fd);
	  char full[] = "We're full right now. Please try again later.\n";
	  write(fd, full, sizeof(full));
	  close(fd);
	}
	pthread_mutex_unlock(&clients->mtx);
      }
    }
  }
  return 0;
}




