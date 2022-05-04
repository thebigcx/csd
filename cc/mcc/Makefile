SRC=main.c lex.c expr.c cg.c
OBJ=$(patsubst %.c, %.o, $(SRC))

BIN=mcc
CFLAGS=

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