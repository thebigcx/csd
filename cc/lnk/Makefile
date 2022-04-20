SRC=main.c bin.c
OBJ=$(patsubst %.c, %.o, $(SRC))

CFLAGS=-g -I..

BIN=lnk

all: $(BIN)

$(BIN): $(OBJ)
	gcc -o $@ $^

%.o: %.c
	gcc -c $< -o $@ $(CFLAGS)

install:
	cp $(BIN) ~/opt/bin/

clean:
	rm $(BIN) $(OBJ)