SRC=readbin.c
TARG=$(patsubst %.c, %, $(SRC))

CFLAGS=-I..

all: $(TARG)

$(TARG): $(SRC)
	gcc -o $@ $< $(CFLAGS)