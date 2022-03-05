SRC=main.c parse.c scan.c
OBJ=$(patsubst %.c, %.o, $(SRC))

BIN=cc

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@