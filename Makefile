CC = cc
CFLAGS = -g -std=gnu99
LDFLAGS = -lpthread


all: canti cantid

canti : client.c
	$(CC) $(CFLAGS) -o canti client.c $(LDFLAGS)

cantid : server.c
	$(CC) $(CFLAGS) -o cantid server.c list.c board.c game.c $(LDFLAGS)

clean :
	rm canti cantid
