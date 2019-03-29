CC=g++
CFLAGS=-I.
DEPS=tree.h functions.h inode.h list.h
OBJ=main.o tree.o functions.o inode.o list.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mirr: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.Phony: clean
clean:
	-rm $(OBJ)
	-rm mirr
