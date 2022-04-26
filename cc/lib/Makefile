SRC=optbl.c
OBJ=$(patsubst %.c, %.o, $(SRC))
TARG=$(patsubst %.o, lib%.a, $(OBJ))

CFLAGS=-g

.PHONY: all clean install

all: $(TARG)

$(TARG): $(OBJ)
	ar rcs $@ $<

$(OBJ): $(SRC)
	gcc -o $@ -c $< $(CFLAGS)

clean:
	rm $(TARG) $(OBJ)

install:
	@for lib in $(TARG) ; do \
		cp $$lib ~/opt/lib/ ; \
	done
