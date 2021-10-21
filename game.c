#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "list.h"
#include "board.h"

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
      LList* squareList = genLegalMovesAtSquare(squareList, pos, i);
      fullList = combineLLists(fullList, squareList);
    }
  }
  /* prunes the list */
  fullList = pruneLegalMoves(fullList, pos);
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
    if(checkLegalMove(itemAtIndex(moves, i))) {
      i++;
    } else /* otherwise, remove it */ {
      removeIndex(moves, i)
    }
  }
  return moves;
}

LList* genPawnMoves(Position* pos, int square) {
  Board board = pos->board;
  assert(board[square]->id == PAWN);
  int color = board[square]->color;
  LList* moves = (LList*)malloc(sizeof(LList));
  initList(moves);

  if(color == WHITE) {
    /* square directly in front of pawn */
    if(board[north(square)]->color == EMPTY) {
      Move m = {square, north(square)}
      pushBackList(moves, &m, sizeof(Move));
    }
    /*
     * TODO: rest of possible pawn moves
     * two square
     * diagonal captures
     * promotion (queen only?)
     */
  } else if(color == BLACK) {
    /* TODO */
  }
  return moves;
}

LList* genKnightMoves(Position* pos, int square) {
  return NULL;
}

LList* genBishopMoves(Position* pos, int square) {
  return NULL;
}

LList* genRookMoves(Position* pos, int square) {
  return NULL;
}

LList* genQueenMoves(Position* pos, int square) {
  return NULL;
}

LList* genKingMoves(Position* pos, int square) {
  return NULL;
}

int checkLegalMove(Move* move, Position* pos) {
  int check = inCheck(pos);
  int toMove = pos->toMove;
  Position* next_pos = genPositionFromMove(move, pos); /* free this pos */
  if(toMove == WHITE) {
    if(check == WHITE) {
      int next_check = inCheck(next_pos);
      if(next_check != WHITE) {
	return 1;
      } else {
	return 0;
      }
    } else {
      return 1;
    }
  } else if(toMove == BLACK) {
    if(check == BLACK) {
      int next_check = inCheck(next_pos);
      if(next_check != BLACK) {
	return 1;
      } else {
	return 0;
      }
    } else {
      return 1;
    }
  }

  return -1;
}

