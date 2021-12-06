#ifndef COMMAND_H
#define COMMAND_H

/*
 * Commands for users in the Game Room
 */
#define C_STARTGAME 0	/* start a new game */
#define C_JOINPLAY 1	/* join a game as a player */
#define C_JOINSPEC 2	/* join a game as a spectator */
#define C_GETGAMES 3	/* get a list of ongoing games */

/*
 * Commands for users playing in a game
 */
#define C_MOVE 8	/* make a move */
#define C_GETMOVES 9	/* get a list of legal moves */
#define C_SENDMSG 10	/* send a message to the other player */
#define C_ASKDRAW 11	/* request a draw */
#define C_RESIGN 12	/* resign the game */
#define C_QUIT 13	/* resign and return to the game room */

/*
 * Commands for users spectating in a game
 */
#define C_STOPSPEC 16	/* stop spectating */

/* Commands for ALL clients */
#define C_DISCONNECT 24

/* other user commands? */

/* Payload Tags */
#define PL_MOVE 0
#define PL_POS 1
#define PL_MSG 2

#define PORT_NUMBER 31415
#endif
