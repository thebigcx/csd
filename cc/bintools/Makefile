SRC=readbin.c
TARG=$(patsubst %.c, %, $(SRC))

CFLAGS=-g -I..

all: $(TARG)

$(TARG): $(SRC)
	gcc -o $@ $< $(CFLAGS)