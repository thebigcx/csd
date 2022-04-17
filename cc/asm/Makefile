SRC=main.c code.c asm.c opcode.c scan.c bin.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g -I..

BIN=asm

.PHONY: all install clean

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)

install:
	cp $(BIN) ~/opt/bin/
	cp optbl.txt ~/opt/share/

clean:
	rm $(BIN) $(OBJ)