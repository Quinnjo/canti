
typedef struct Position {
  /* Two 64 integer arrays
   * pieces stores the piece ids
   * colors stores the piece colors
   * both indexed as declared in piece.h
   */
  int pieces[64];
  int colors[64];
  int toMove; /* 1 for white, 2 for black */
  /* true (1) or false (0) values */
  int whiteCastlingRights;
  int blackCastlingRights;
  
  /* the square on which ep can happen
   * -1 if not possible (most of the time) */
  int enPassantSquare;
} Position;

typedef struct Game {
  Position* board;
  /* some way to keep past positions */
  /* some way to keep past moves */
  int timeStart;
  int timeWhite;
  int timeBlack;
} Game;

/*
 * We need to keep track of:
 * 1) The current position on the board
 * 2) The castling rights of both players
 * 3) Every move made thus far
 * 4) Every position had thus far (draw by repetition)
 * 	- Hashed positions in a linked list
 * 	- Only keep the positions after the last "irreversible move"
 * 		- capture, castle, pawn move
 * 5) The time elapsed for both players
 * 
 * Split this into a few types:
 * 	- The Position type will hold a position with
 * 	  all of its relevant information
 * 	- The Game type will hold the rest: past moves,
 * 	  past positions, game clock, other info
 */


