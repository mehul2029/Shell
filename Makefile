CC=gcc
CFLAGS=-I.
DEPS = cmd.h history.h linkedlist.h
OBJ = cmd.o history.o linkedlist.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
