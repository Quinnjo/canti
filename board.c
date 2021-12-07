#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "board.h"


/* 
 * Starting Board
 * startPieces has piece information
 * startColors has color information
 */

int startPieces[64] = {
  ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK,
  PAWN, PAWN,   PAWN,   PAWN,  PAWN, PAWN,   PAWN,   PAWN,
  EMPTY,EMPTY,  EMPTY,  EMPTY, EMPTY,EMPTY,  EMPTY,  EMPTY, 
  EMPTY,EMPTY,  EMPTY,  EMPTY, EMPTY,EMPTY,  EMPTY,  EMPTY,
  EMPTY,EMPTY,  EMPTY,  EMPTY, EMPTY,EMPTY,  EMPTY,  EMPTY,
  EMPTY,EMPTY,  EMPTY,  EMPTY, EMPTY,EMPTY,  EMPTY,  EMPTY,
  PAWN, PAWN,   PAWN,   PAWN,  PAWN, PAWN,   PAWN,   PAWN,
  ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
};

int startColors[64] = {
  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
  WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY,
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
  BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK
};

int valid(int square) {
  return 0 <= square && square < 64;
}

int rank(int square) {
  assert(valid(square));
  return square / 8 + 1;
}

int file(int square) {
  assert(valid(square));
  return square % 8 + 1;
}

/* return -1 on error */
int squareFromString(char* str) {
  char c = tolower(str[0]);
  if(!(isalpha(c) && c <= 'h' && isdigit(str[1]))) {
    return -1;
  }
  int r = str[1] - '0';
  if(!(1 <= r && r <= 8)) {
    return -1;
  }
  return squareFromCoords(c - 'a' + 1, r);
}

/*
 * puts the result in the passed buffer str
 * str should have enough space for 2 characters
 */
void squareToString(char str[2], int square) {
  assert(valid(square));
  int f = file(square);
  int r = rank(square);
  str[0] = f - 1 + 'a';
  str[1] = r + '0';
}

int squareFromCoords(int file, int rank) {
  assert(1 <= file && file <= 8);
  assert(1 <= rank && rank <= 8);
  return (rank - 1) * 8 + (file - 1);
}

int isLight(int square) {
  assert(valid(square));
  return square % 2 == rank(square) % 2;
}

int isDark(int square) {
  assert(valid(square));
  return square % 2 == square / 8 % 2;
}

int north(int square) {
  if(!valid(square) || square >= 56) {
    return -1;
  }
  return square + 8;
}

int east(int square) {
  if(!valid(square) || (square - 7) % 8 == 0) {
    return -1;
  }
  return square + 1;
}

int south(int square) {
  if(!valid(square) || square <= 7) {
    return -1;
  }
  return square - 8;
}

int west(int square) {
  if(!valid(square) || square % 8 == 0) {
    return -1;
  }
  return square - 1;
}

int northeast(int square) {
  return north(east(square));
}

int southeast(int square) {
  return south(east(square));
}

int southwest(int square) {
  return south(west(square));
}

int northwest(int square) {
  return north(west(square));
}

char pieceChar(Piece* piece) {
  char c;
  switch(piece->id) {
    case KING:
      c = 'k';
      break;
    case QUEEN:
      c = 'q';
      break;
    case BISHOP:
      c = 'b';
      break;
    case KNIGHT:
      c = 'n';
      break;
    case ROOK:
      c = 'r';
      break;
    case PAWN:
      c = 'p';
      break;
    case EMPTY:
      c = ' ';
      break;
    default:
      return '?';
      break;
  }

  if(piece->color == 2) {
    c = c - 'a' + 'A';
  }

  return c;
}

char* pieceToString(Piece* p) {
  switch(p->id) {
    case KING:
      return "King";
      break;
    case QUEEN:
      return "Queen";
      break;
    case BISHOP:
      return "Bishop";
      break;
    case KNIGHT:
      return "Knight";
      break;
    case ROOK:
      return "Rook";
      break;
    case PAWN:
      return "Pawn";
      break;
    default:
      return "Unknown";
      break;
  }
}

char pieceToChar(Piece* p) {
  switch(p->id) {
    case KING:
      return 'K';
      break;
    case QUEEN:
      return 'Q';
      break;
    case BISHOP:
      return 'B';
      break;
    case KNIGHT:
      return 'N';
      break;
    case ROOK:
      return 'R';
      break;
    case PAWN:
      /* The Pawn does not have its own unique character */
    default:
      return 'X';
  }
}

int charToPieceID(char c) {
  switch(c) {
    case 'K':
      return KING;
      break;
    case 'Q':
      return QUEEN;
      break;
    case 'B':
      return BISHOP;
      break;
    case 'N':
      return KNIGHT;
      break;
    case 'R':
      return ROOK;
      break;
    default:
      return -1;
  }
}

void makeBoard(Board board) {
  for(int i = 0; i < 64; i++) {
    board[i] = (Piece*)malloc(sizeof(Piece));
    board[i]->id = startPieces[i];
    board[i]->color = startColors[i];
  }
}

void freeBoard(Board board) {
  for(int i = 0; i < 64; i++) {
    free(board[i]);
  }
}

/* prints simple board from white's perspective */
void printBoardSimple(Board board) {
  printf("Simple Board:\n\n");
  for(int r = 7; r >= 0; r--) {
    printf("\t%c\t", '1'+r); /* rank number */
    for(int c = 0; c < 8; c++) {
      char p = pieceChar(board[8*r+c]);
      p = (p == ' ') ? '.' : p;
      printf("%c ", p);
    }
    printf("\n");
  }
  printf("\n\t\t");
  for(int i = 0; i < 8; i++) {
    printf("%c ", 'a'+i); /* file letter */
  }
  printf("\n");
}


char* board_white() {
  char* buf = malloc(BOARD_STRLEN);
  FILE* f = fopen("./board_white.txt", "r");
  int bytes_read = fread(buf, sizeof(char), BOARD_STRLEN, f);
  //printf("%d\n", bytes_read);
  //printf("%s\n", buf);
  fclose(f);
  return buf;
}

char* board_black() {
  char* buf = malloc(BOARD_STRLEN);
  FILE* f = fopen("./board_black.txt", "r");
  int bytes_read = fread(buf, sizeof(char), BOARD_STRLEN, f);
  //printf("%d\n", bytes_read);
  //printf("%s\n", buf);
  fclose(f);
  return buf;
}

void board_white_buf(char buf[BOARD_STRLEN]) {
  FILE* f = fopen("./board_white.txt", "r");
  int bytes_read = fread(buf, sizeof(char), BOARD_STRLEN, f);
  fclose(f);
}

void board_black_buf(char buf[BOARD_STRLEN]) {
  FILE* f = fopen("./board_black.txt", "r");
  int bytes_read = fread(buf, sizeof(char), BOARD_STRLEN, f);
  fclose(f);
}

/*
 * TODO: Clean up magic numbers in these functions
 */
char* boardStrWhite(Board board) {
  char* str = board_white();
  for(int r = 0; r < 8; r++) {
    for(int c = 0; c < 8; c++) {
      str[655-86*r + 4*c] = pieceChar(board[8*r+c]);
    }
  }
  return str;
}

char* boardStrBlack(Board board) {
  char* str = board_black();
  for(int r = 0; r < 8; r++) {
    for(int c = 0; c < 8; c++) {
      str[81+86*r - 4*c] = pieceChar(board[8*r+c]);
    }
  }
  return str;
}

void boardToBufWhite(Board board, char buf[BOARD_STRLEN]) {
  board_white_buf(buf);
  for(int r = 0; r < 8; r++) {
    for(int c = 0; c < 8; c++) {
      buf[655-86*r + 4*c] = pieceChar(board[8*r+c]);
    }
  }
  buf[BOARD_STRLEN-1] = 0;
}

void boardToBufBlack(Board board, char buf[BOARD_STRLEN]) {
  board_black_buf(buf);
  for(int r = 0; r < 8; r++) {
    for(int c = 0; c < 8; c++) {
      buf[81+86*r - 4*c] = pieceChar(board[8*r+c]);
    }
  }
  buf[BOARD_STRLEN-1] = 0;
}


