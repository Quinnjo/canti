#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <errno.h>
#include <assert.h>

#include "game.h"
#include "list.h"
#include "board.h"
#include "command.h"

void checkError(int status,int line) {
  if (status < 0) {
    printf("socket error(%d)-%d: [%s]\n",getpid(),line,strerror(errno));
    exit(-1);
  }
}

/* 
 * Writes sz bytes of data to file descriptor fd
 * Returns number of bytes sent (equal to sz on success)
 */
/*
int sendBinary(int fd, void* data, int sz) {
  int sent = 0;
  int status;
  while(sz) {
    status = write(fd, data+sent, sz);
    sent += status;
    sz -= status;
  }
  return sent;
}
*/

/* reads a string from stdin, returns after reaching \n\0 */
char* readString() {
  int sz = 2;
  char* str = malloc(sz);
  int i = 0;
  char c;
  while((c = getchar()) != '\n' && c != EOF) {
    if(i == sz - 2) {
      sz *= 2;
      str = realloc(str, sz);
    }
    str[i++] = c;
  }
  if(c == EOF) {
    free(str);
    return NULL;
  } else {
    str[i] = c;
    str[i+1] = 0;
    return str;
  }
}

/* currently UNUSED */
/*
int handleCommand(char* input) {
  int sz = 2;
  char** args = malloc(sz);
  int i = 0;
  char* token;
  while((a = strtok(input, " "))) {
    if(i == sz - 1) {
      sz *= 2;
      args = realloc(args, sz);
    }
    args[i] = token;
  }
  return 0;
}
*/

int main(int argc, char* argv[]) {
  int debug = 0;

  if(argc < 2) {
    printf("Usage: ./<program> <hostname> [debug]\n");
    return 1;
  }
  if(argc > 2) {
    debug = 1;
    printf("debug mode on!\n");
  }

  char* hostname = argv[1];

  int sid = socket(PF_INET,SOCK_STREAM,0);
  struct sockaddr_in srv;
  struct hostent *server = gethostbyname(hostname);
  srv.sin_family = AF_INET;
  srv.sin_port = htons(PORT_NUMBER);
  memcpy(&srv.sin_addr.s_addr,server->h_addr,server->h_length);
  int status = connect(sid,(struct sockaddr*)&srv,sizeof(srv));
  checkError(status,__LINE__);

  char buf[1024];
  int done = 0;
  int checkInput = 1;

  do {
    fd_set fdin;
    FD_ZERO(&fdin);
    FD_SET(sid, &fdin);
    if(checkInput) {
      FD_SET(0, &fdin);
    }

    int selectResult = select(sid+1, &fdin, NULL, NULL, NULL);

    if(selectResult > 0) {

      /* check stdin */
      if(FD_ISSET(0, &fdin)) {
	/* we have something to send to the server */
	char* command = readString();
	if(command == NULL) {
	  checkInput = 0;
	  continue;
	}
	int wb = write(sid, command, strlen(command)/*+1*/); //send the null byte
	if(debug) {
	  printf("Sending command: %s", command);
	}
	free(command);
      } else if(FD_ISSET(sid, &fdin)) {
	/* we must receive a message from the server */
	int rb = read(sid, buf, sizeof(buf));
	done = rb == 0;
	if(!done) {
	  write(1, buf, rb);
	}
      }
    }
  } while(!done);

  close(sid);
  return 0;
}
