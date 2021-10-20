#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "list.h"



LList* genLegalMovesAtSquare(LList* moves, Position* pos, int square) {
  assert(moves && lengthList(moves) == 0);
  assert(pos && valid(square));

  /* if the square has a piece which the current player can move */
  if(pos->board[square]->color == pos->toMove) {
    switch(pos->board[square]->id) {
      case PAWN: return genPawnMoves(moves, pos, square);
      case KNIGHT: return genKnightMoves(moves, pos, square);
      case BISHOP: return genBishopMoves(moves, pos, square);
      case ROOK: return genRookMoves(moves, pos, square);
      case QUEEN: return genQueenMoves(moves, pos, square);
      case KING: return genKingMoves(moves, pos, square);
      default: return NULL; /* the piece is invalid */
    }
  }
  return NULL; /* the player can't move on this square */
}
