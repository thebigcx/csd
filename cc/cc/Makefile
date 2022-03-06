SRC=main.c stmt.c expr.c scan.c cg.c type.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g

BIN=cc

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)