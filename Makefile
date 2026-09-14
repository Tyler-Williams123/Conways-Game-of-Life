CC = gcc
Output = game.exe
SRC = Game.c glad/src/gl.c
CFLAGS = -Iglad/include -Wextra -Wall
LIBS = -lglfw3 -lopengl32

$(Output): $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(Output) $(LIBS)