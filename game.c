#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "game.h"
#include "list.h"
#include "board.h"

Position* newPosition() {
  Board board = (Board)malloc(sizeof(Piece)*64);
  makeBoard(board);
  Position* pos = (Position*)malloc(sizeof(Position));
  pos->board = board;
  pos->toMove = WHITE;
  return pos;
}

void freePosition(Position* pos) {
  freeBoard(pos->board);
  free(pos->board);
}

LList* genLegalMovesNoPrune(Position* pos) {
  assert(pos);
  LList* fullList = (LList*)malloc(sizeof(LList));
  initList(fullList);
  int toMove = pos->toMove;
  /*
   * proceed through the board
   * if the square has that color piece on it, call genLegalMovesAtSquare
   */
  for(int i = 0; i < 64; i++) {
    if(pos->board[i]->color == toMove) {
      LList* squareList = genLegalMovesAtSquare(pos, i);
      fullList = combineLLists(fullList, squareList);
    }
  }
  return fullList;
}

LList* genLegalMoves(Position* pos) {
  assert(pos);
  LList* fullList = (LList*)malloc(sizeof(LList));
  initList(fullList);
  int toMove = pos->toMove;
  /*
   * proceed through the board
   * if the square has that color piece on it, call genLegalMovesAtSquare
   */
  for(int i = 0; i < 64; i++) {
    if(pos->board[i]->color == toMove) {
      LList* squareList = genLegalMovesAtSquare(pos, i);
      fullList = combineLLists(fullList, squareList);
    }
  }
  /* prunes the list */
  fullList = pruneLegalMoves(fullList, pos);
  return fullList;
}

LList* genLegalMovesAtSquare(Position* pos, int square) {
  assert(pos && valid(square));

  /* if the square has a piece which the current player can move */
  if(pos->board[square]->color == pos->toMove) {
    switch(pos->board[square]->id) {
      case PAWN: return genPawnMoves(pos, square);
      case KNIGHT: return genKnightMoves(pos, square);
      case BISHOP: return genBishopMoves(pos, square);
      case ROOK: return genRookMoves(pos, square);
      case QUEEN: return genQueenMoves(pos, square);
      case KING: return genKingMoves(pos, square);
      default: return NULL; /* the piece is invalid */
    }
  }
  return NULL; /* the player can't move on this square */
}

/*
 * return moves, the same list, after pruning
 */
LList* pruneLegalMoves(LList* moves, Position* pos) {
  int i = 0;
  while(i < lengthList(moves)) {
    /* if the move is legal, skip it */
    if(moveIsSafe((Move*)itemAtIndex(moves, i), pos)) {
      i++;
    } else /* otherwise, remove it */ {
      removeIndex(moves, i);
    }
  }
  return moves;
}

int numberLegalMoves(Position* pos) {
  LList* moves = genLegalMoves(pos);
  int n = moves->len;
  freeList(moves);
  free(moves);
  return n;
}

LList* genPawnMoves(Position* pos, int square) {
  Board board = pos->board;
  assert(board[square]->id == PAWN);
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  if(color == WHITE) {
    /* square directly in front of pawn */
    if(valid(north(square)) && board[north(square)]->color == EMPTY) {
      Move m = {square, north(square)};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* two square if on second rank */
    if(rank(square) == 2 && board[north(square)]->color == EMPTY && board[north(north(square))]->color == EMPTY) {
      Move m = {square, north(north(square))};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* northeast capture */
    if(valid(northeast(square)) && board[northeast(square)]->color == BLACK) {
      Move m = {square, northeast(square)};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* northwest capture */
    if(valid(northwest(square)) && board[northwest(square)]->color == BLACK) {
      Move m = {square, northwest(square)};
      pushBackList(moves, &m, sizeof(Move));
    }
  } else if(color == BLACK) {
    /* square in front of pawn */
    if(valid(south(square)) && board[south(square)]->color == EMPTY) {
      Move m = {square, south(square)};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* two squares if on second rank */
    if(rank(square) == 7 && board[south(square)]->color == EMPTY && board[south(south(square))]->color == EMPTY) {
      Move m = {square, south(south(square))};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* southeast capture */
    if(valid(southeast(square)) && board[southeast(square)]->color == WHITE) {
      Move m = {square, southeast(square)};
      pushBackList(moves, &m, sizeof(Move));
    }

    /* southwest capture */
    if(valid(southwest(square)) && board[southwest(square)]->color == WHITE) {
      Move m = {square, southwest(square)};
      pushBackList(moves, &m, sizeof(Move));
    }
  }
  return moves;
}

LList* genKnightMoves(Position* pos, int square) {
  Board board = pos->board;
  assert(board[square]->id == KNIGHT);
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  /* clockwise, starting with northnortheast */

  /* northnortheast */
  if(valid(north(northeast(square))) && board[north(northeast(square))]->color != color) {
    Move m = {square, north(northeast(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* eastnortheast */
  if(valid(east(northeast(square))) && board[east(northeast(square))]->color != color) {
    Move m = {square, east(northeast(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* eastsoutheast */
  if(valid(east(southeast(square))) && board[east(southeast(square))]->color != color) {
    Move m = {square, east(southeast(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* southsoutheast */
  if(valid(south(southeast(square))) && board[south(southeast(square))]->color != color) {
    Move m = {square, south(southeast(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* southsouthwest */
  if(valid(south(southwest(square))) && board[south(southwest(square))]->color != color) {
    Move m = {square, south(southwest(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* westsouthwest */
  if(valid(west(southwest(square))) && board[west(southwest(square))]->color != color) {
    Move m = {square, west(southwest(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* westnorthwest */
  if(valid(west(northwest(square))) && board[west(northwest(square))]->color != color) {
    Move m = {square, west(northwest(square))};
    pushBackList(moves, &m, sizeof(Move));
  }

  /* northnorthwest */
  if(valid(north(northwest(square))) && board[north(northwest(square))]->color != color) {
    Move m = {square, north(northwest(square))};
    pushBackList(moves, &m, sizeof(Move));
  }
  return moves;
}

LList* genBishopMoves(Position* pos, int square) {
  Board board = pos->board;
  //assert(board[square]->id == BISHOP);
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  int test;

  /* northeast */
  test = northeast(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break; /* if we run into an opposite piece */
      }
      test = northeast(test);
    } else {
      break; /* we ran into a piece of the same color */
    }
  }

  /* southeast */
  test = southeast(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = southeast(test);
    } else {
      break;
    }
  }

  /* southwest */
  test = southwest(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = southwest(test);
    } else {
      break;
    }
  }
  /* northwest */
  test = northwest(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = northwest(test);
    } else {
      break;
    }
  }
  return moves;
}

LList* genRookMoves(Position* pos, int square) {
  Board board = pos->board;
  //assert(board[square]->id == ROOK);
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  int test;

  /* north */
  test = north(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = north(test);
    } else {
      break;
    }
  }

  /* east */
  test = east(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = east(test);
    } else {
      break;
    }
  }

  /* south */
  test = south(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = south(test);
    } else {
      break;
    }
  }
  /* west */
  test = west(square);
  while(valid(test)) {
    if(board[test]->color != color) {
      Move m = {square, test};
      pushBackList(moves, &m, sizeof(Move));
      if(board[test]->color != EMPTY) {
	break;
      }
      test = west(test);
    } else {
      break;
    }
  }
  return moves;
}

LList* genQueenMoves(Position* pos, int square) {
  /* A Queen is a hybrid of a bishop and a rook */
  LList* bish = genBishopMoves(pos, square);
  LList* rook = genRookMoves(pos, square);
  LList* moves = combineLLists(bish, rook);
  return moves;
}

LList* genKingMoves(Position* pos, int square) {
  Board board = pos->board;
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  /* check one square in each direction */
  int sq[8] = {
    north(square), northeast(square), east(square), southeast(square),
    south(square), southwest(square), west(square), northwest(square)
  };

  for(int i = 0; i < 8; i++) {
    if(valid(sq[i]) && board[sq[i]]->color != color) {
      Move m = {square, sq[i]};
      pushBackList(moves, &m, sizeof(Move));
    }
  }
  return moves;
}

int moveIsSafe(Move* move, Position* pos) {
  int check = inCheck(pos);
  int toMove = pos->toMove;
  Position* next_pos = genPositionFromMove(move, pos); /* free this pos */
  int next_check = inCheck(next_pos);
  freePosition(next_pos);
  free(next_pos);
  if(toMove == WHITE) {
    if(next_check & WHITE) {
      return 0;
    }
    return 1;
  } else if(toMove == BLACK) {
    if(next_check & BLACK) {
      return 0;
    }
    return 1;
  }
  return -1;
}

int moveIsLegal(Move* move, Position* pos) {
  LList* list = genLegalMoves(pos);
  int result = moveInList(list, move);
  freeList(list);
  free(list);
  return result;
}

int moveInList(LList* list, Move* move) {
  LLNode* cur = list->head;
  for(int i = 0; i < lengthList(list); i++) {
    Move* m = (Move*)cur->data;
    if(m->start == move->start && m->end == move->end) {
      return 1;
    }
    cur = cur->next;
  }
  return 0;
}

/* 
 * Does not change the given Position
 * Allocates a new position that must be free'd by caller
 */
Position* genPositionFromMove(Move* move, Position* pos) {
  Position* new = newPosition();
  new->toMove = (pos->toMove == WHITE) ? BLACK : WHITE; /* switch who moves */
  for(int i = 0; i < 64; i++) {
    memcpy(new->board[i], pos->board[i], sizeof(Piece));
  }
  Board board = new->board;
  board[move->end]->color = board[move->start]->color;
  board[move->end]->id = board[move->start]->id;
  board[move->start]->id = EMPTY;
  board[move->start]->color = EMPTY;
  return new;
}

/*
 * Changes the given position
 * Does not allocate any new memory
 */
void applyMoveToPosition(Move* move, Position* pos) {
  pos->toMove = (pos->toMove == WHITE) ? BLACK : WHITE;
  Board board = pos->board;
  board[move->end]->color = board[move->start]->color;
  board[move->end]->id = board[move->start]->id;
  board[move->start]->id = EMPTY;
  board[move->start]->color = EMPTY;
}


int inCheck(Position* pos) {
  int checkVal = 0;
  int og_toMove = pos->toMove;

  /* check WHITE */
  pos->toMove = BLACK;
  if(inCheckWhite(pos)) {
    checkVal += WHITE;
  }

  /* check BLACK */
  pos->toMove = WHITE;
  if(inCheckBlack(pos)) {
    checkVal += BLACK;
  }

  /* reset original toMove value, return who is in check */
  pos->toMove = og_toMove;
  return checkVal;
}

/* 
 * Returns the number of pieces that attack the white king
 * toMove MUST be BLACK for this function to work
 */
int inCheckWhite(Position* pos) {
  assert(pos->toMove == BLACK);
  int checkVal = 0;

  /* find the square with the white king */
  int king_square = -1;
  for(int i = 0; i < 64; i++) {
    if(pos->board[i]->id == KING && pos->board[i]->color == WHITE) {
      king_square = i; break;
    }
  }
  assert(valid(king_square));

  /* check for moves that attack the king square */
  LList* moves = genLegalMovesNoPrune(pos);
  for(int i = 0; i < lengthList(moves); i++) {
    if(((Move*)itemAtIndex(moves, i))->end == king_square) {
      checkVal++;
    }
  }
  freeList(moves);
  free(moves);
  return checkVal;
}

/*
 * Same functionality as inCheckWhite
 * colors reversed
 */
int inCheckBlack(Position* pos) {
  assert(pos->toMove == WHITE);
  int checkVal = 0;

  /* find square with black king */
  int king_square = -1;
  for(int i = 0; i < 64; i++) {
    if(pos->board[i]->id == KING && pos->board[i]->color == BLACK) {
      king_square = i; break;
    }
  }
  assert(valid(king_square));

  /* check for moves that attack the king square */
  LList* moves = genLegalMovesNoPrune(pos);
  for(int i = 0; i < lengthList(moves); i++) {
    if(((Move*)itemAtIndex(moves, i))->end == king_square) {
      checkVal++;
    }
  }
  freeList(moves);
  free(moves);
  return checkVal;
}



