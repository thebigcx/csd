SRC=main.c code.c asm.c opcode.c scan.c bin.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g -I..

BIN=asm

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)