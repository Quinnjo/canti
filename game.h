#ifndef GAME_H
#define GAME_H

#include "list.h"

typedef struct Position {
  /* 
   * Board defined in board.h
   * board[n] returns Piece* at square n
   * EX: board[3]->color returns the color of the piece on d1
   */
  Board board;
  int toMove; /* 1 for white, 2 for black */
  /* true (1) or false (0) values */
  int whiteCastlingRights;
  int blackCastlingRights;
  
  /* the square on which ep can happen
   * -1 if not possible (most of the time) */
  int enPassantSquare;
} Position;

typedef struct Game {
  Position* position;
  /* 
   * past positions can be constructed from moves
   * moves with even index were played by white
   * moves with odd index were played by black
   * use last_irreversible_move for threefold repitition
   */
  LList* moves;
  int last_irreversible_move;

  int timeStart;
  int timeWhite;
  int timeBlack;
} Game;

typedef struct Move {
  int start;
  int end;
}

/*
 * We need to keep track of:
 * 1) The current position on the board
 * 2) The castling rights of both players
 * 3) Every move made thus far
 * 4) Every position had thus far (draw by repetition)
 * 	- Reconstruct positions from past moves
 * 	- Only check the positions after the last "irreversible move"
 * 		- capture, castle, pawn move
 * 5) The time elapsed for both players
 * 
 * Split this into types:
 * 	- The Game type will hold the rest: past moves,
 * 	  past positions, game clock, other info
 */

/*
 * Generate all the legal moves that can be played in position
 * Receives a LList* of Move* to place legal moves in, returns when done
 * Receives a Position*
 */
LList* genLegalMoves(LList* moves, Position* pos);
LList* genLegalMovesAtSquare(LList* moves, Position* pos);
/*
 * Generate all the legal moves for a specific piece on a square
 * Must receive an empty LList, the caller should free the list when finished
 */
LList* genPawnMoves(LList* moves, Position* pos, int square);
LList* genKnightMoves(LList* moves, Position* pos, int square);
LList* genBishopMoves(LList* moves, Position* pos, int square);
LList* genRookMoves(LList* moves, Position* pos, int square);
LList* genQueenMoves(LList* moves, Position* pos, int square);
LList* genKingMoves(LList* moves, Position* pos, int square);

/* return 1 if move is legal, 0 otherwise */
int checkLegalMove(Move* move, Position* pos);




#endif
