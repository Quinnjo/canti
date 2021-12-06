#ifndef GAME_H
#define GAME_H

#include "list.h"
#include "board.h"

typedef struct Position {
  /* 
   * Board defined in board.h
   * board[n] returns Piece* at square n
   * EX: board[3]->color returns the color of the piece on d1
   */
  Board board;
  int toMove; /* WHITE or BLACK */

  // NOT IMPLEMENTED IN CURRENT VERSION
  /* true (1) or false (0) values */
  //int whiteCastlingRights;
  //int blackCastlingRights;
  
  /* the square on which ep can happen
   * -1 if not possible (most of the time) */
  //int enPassantSquare;
} Position;

typedef struct Move {
  int start;
  int end;
} Move;
/*
 * allocates and initializes a new position
 */
Position* newPosition();

/*
 * frees pos's attributes
 * does not free the pos pointer itself
 */
void freePosition(Position* pos);

/*
 * returns a new position made from performing the move, legal or otherwise
 * caller should free the return position
 */
Position* genPositionFromMove(Move* move, Position* pos);
void applyMoveToPosition(Move* move, Position* pos);

/*
 * return 0 if neither player in check
 * return WHITE if white in check
 * return BLACK if black in check
 * return WHITE + BLACK if both in check
 * return -1 on error
 */
int inCheck(Position* pos);
int inCheckWhite(Position* pos);
int inCheckBlack(Position* pos);

/*
 * Generate all the legal moves that can be played in position
 * Allocates a new LList of moves that the caller must free
 * Receives a Position*
 *
 * pruneLegalMoves removes moves that would place the player in check or
 * fail to remove them from check
 */
LList* genLegalMoves(Position* pos);
LList* genLegalMovesNoPrune(Position* pos);
LList* genLegalMovesAtSquare(Position* pos, int square);
LList* pruneLegalMoves(LList* moves, Position* pos);
int numberLegalMoves(Position* pos);
/*
 * Generate all the legal moves for a specific piece on a square
 * Creates a new list that the caller should free when finished
 * Does not account for checks -- these moves should be pruned with pruneLegalMoves()
 */
LList* genPawnMoves(Position* pos, int square);
LList* genKnightMoves(Position* pos, int square);
LList* genBishopMoves(Position* pos, int square);
LList* genRookMoves(Position* pos, int square);
LList* genQueenMoves(Position* pos, int square);
LList* genKingMoves(Position* pos, int square);

/* return 1 if move does not endanger the king, 0 otherwise */
int moveIsSafe(Move* move, Position* pos);
int moveIsLegal(Move* move, Position* pos);

/* return 1 if move is in list (list must contain only moves), 0 otherwise */
int moveInList(LList* list, Move* move);




#endif
