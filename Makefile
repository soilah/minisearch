IDIR = include
SRC = src
CC = gcc
CFLAGS = -I$(IDIR) -Wall -Wextra -O2 -g -ggdb
OBJ = main.o structs.o minisearch.o
LIBS = -lm

minisearch: $(OBJ)
	gcc -o minisearch $(OBJ) $(LFLAGS) $(LIBS)

main.o : ${SRC}/main.c
	gcc -c $^ $(CFLAGS)

structs.o : ${SRC}/structs.c
	gcc -c $^ $(CFLAGS)

minisearch.o : ${SRC}/minisearch.c
	gcc -c $^ $(CFLAGS)

clean:
	${RM} minisearch *.o
