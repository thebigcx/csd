SRC=main.c lex.c expr.c stmt.c sym.c cg.c
OBJ=$(patsubst %.c, %.o, $(SRC))

BIN=mcc
CFLAGS=-g

.PHONY: all clean install

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -o $@ -c $< $(CFLAGS)

install:
	cp $(BIN) ~/opt/bin/

clean:
	rm $(OBJ) $(BIN)
