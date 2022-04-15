SRC=main.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g -I..

BIN=lnk

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)