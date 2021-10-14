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

/*
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
*/

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

int squareFromString(char* str) {
  char c = tolower(str[0]);
  assert(isalpha(c) && c <= 'h');
  assert(isdigit(str[1]));
  int r = str[1] - '0';
  assert(1 <= r && r <= 8);
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

char pieceChar(int piece, int color) {
  char c;
  switch(piece) {
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

  if(color == 2) {
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

/* prints simple board from white's perspective */
void printBoardSimple(int pieces[64], int colors[64]) {
  printf("Simple Board:\n\n");
  for(int r = 7; r >= 0; r--) {
    printf("%c\t", '1'+r);
    for(int c = 0; c < 8; c++) {
      char p = pieceChar(pieces[8*r+c], colors[8*r+c]);
      p = (p == ' ') ? '.' : p;
      printf("%c ", p);
    }
    printf("\n");
  }
  printf("\n\t");
  for(int i = 0; i < 8; i++) {
    printf("%c ", 'a'+i);
  }
  printf("\n");
}


