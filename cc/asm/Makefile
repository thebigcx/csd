SRC=main.c code.c asm.c opcode.c scan.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g

BIN=asm

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)