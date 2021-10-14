
/*
 * Contains the types used for the game
 */

#define EMPTY 0
#define PAWN 1
#define KNIGHT 2
#define BISHOP 3
#define ROOK 4
#define QUEEN 5
#define KING 6

#define WHITE 1
#define BLACK 2

#define A 1
#define B 2
#define C 3
#define D 4
#define E 5
#define F 6
#define G 7
#define H 8

/*
 * The squares of the chess board are numbered from 0 to 63
 * a1 is 0, a2 is 1, a3 is 2, ... , a8 is 7
 * b1 is 8, b2 is 9, ...
 * f8 is 61, g8 is 62, h8 is 63
 */ 

/*
 * For a square with number n the following relations hold:
 *
 * rank(n) 	= 	n / 8 + 1
 * file(n) 	= 	n % 8 + 1
 * light(n) 	=	n % 2 == rank(n) % 2
 * dark(n)	=	n % 2 == n / 8 % 2
 */

/*
 * Compass:
 *
 *    NW	 N	  NE
 *	+7	+8	+9
 *		 _
 *    W	-1	|0|	+1 E
 *		---
 *	-9	-8	-7
 *    SW	 S	  SE
 *
 */


typedef struct Piece {
  int id;
  int color;
  int square;
} Piece;

typedef struct Move {
  Piece* piece;
  int start_square;
  int end_square;

  /* Other potential fields:
   * Number
   * Flags (e.p.)
   */

} Move;

/*
 * Return 1 if square is valid (0 <= square < 64)
 * Return 0 otherwise
 */
int valid(int square);

/*
 * Return the rank/file given a square
 */
int rank(int square);
int file(int square);

int squareFromString(char* str);
void squareToString(char str[2], int square);
int squareFromCoords(int file, int rank);

/*
 * Return if a square is light/dark
 */
int isLight(int square);
int isDark(int square);

/*
 * Return the next square in the direction, or -1 if not possible
 */
int north(int square);
int northeast(int square);
int east(int square);
int southeast(int square);
int south(int square);
int southwest(int square);
int west(int square);
int northwest(int square);

char pieceChar(int piece, int color);
char* pieceToString(Piece* p);
char pieceToChar(Piece* p);
int charToPieceID(char c);

void printBoardSimple(int pieces[64], int colors[64]);




