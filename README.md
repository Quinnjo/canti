# Canti

Canti is a client-server chess application written in C. Canti was developed to run on Unix systems and uses the pthread API.


## Usage
To compile, run `make` in the terminal. Two executables, `canti` and `canitid` will be created in the directory.
The client program is `canti`. The command `./canti <hostname> [debug]` will run the program connecting to the given hostname, and takes an optional debug argument. The client can use a number of commands once connected.

**For clients in the Hub Room (immediately after connecting):**
* `newgame` — create a new game
* `listgames` — list all games that the server is handling with an ID, player count, and spectator count
* `joinplay [ID]` — join the game with the specified ID, or the next available game
* `joinspec <ID>` — spectate the game with the specified ID
* `disconnect` — disconnect from the server

**For clients playing in a game:**
* `move <square1><square2>` — move a piece from square1 to square2
* `listmoves` — list the legal moves the player can perform
* `message <message>` — send a message to the opponent and any spectators
* `resign` — resign (forfeit) the game and disconnect

**For clients spectating a game:**
* `disconnect` — disconnect from the game and server

To start a server, run `./cantid [debug]` with an optional debug argument. The server will automatically handle incoming connections and create the appropriate threads.

## File Descriptions
* `server.c` — contains the server code
* `client.c` — contains the client code
* `board.c` — contains the board logic and data structures
* `game.c` — contains functions to read information from and edit the board data structures
* `list.c` — a generic linked list implementation
* `command.h` — contains constants (port number)
* `board_white.txt` and `board_black.txt` — text representations of a chess board
