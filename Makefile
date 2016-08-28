CC=gcc
CFLAGS=-I.
DEPS = cmd.h history.h linkedlist.h builtin.h
OBJ = cmd.o history.o linkedlist.o builtin.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

shell: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
