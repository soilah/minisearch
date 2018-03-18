IDIR = ../Include
CC = gcc
CFLAGS = -I$(IDIR) -g -ggdb
OBJ = main.o structs.o minisearch.o
LIBS = -lm

minisearch: $(OBJ)
	gcc -o minisearch $(OBJ)  $(CFLAGS) $(LIBS) -g3


main.o : main.c
	gcc -c main.c $(CFLAGS)

structs.o : structs.c
	gcc -c structs.c $(CFLAGS)

minisearch.o : minisearch.c
	gcc -c minisearch.c $(CFLAGS)

clean:
	rm minisearch *.o


