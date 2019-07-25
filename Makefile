CC=gcc
CFLAGS=-I. -lstdc++
DEPS = C1581.h
OBJ = C1581.o main.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

1581emu: $(OBJ)
	$(CC) -g -o $@ $^ $(CFLAGS)