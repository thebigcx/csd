SRC=readbin.c
TARG=$(patsubst %.c, %, $(SRC))

CFLAGS=-g -I..

all: $(TARG)

$(TARG): $(SRC)
	gcc -o $@ $< $(CFLAGS)

install:
	@for bin in $(TARG) ; do \
		cp $$bin ~/opt/bin/ ; \
	done

clean:
	rm $(TARG)
