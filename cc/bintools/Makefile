TARGETS=disas readbin
CFLAGS=-g -I..

all: $(TARGETS)

disas: disas.c
	gcc -o $@ $< $(CFLAGS) -L../lib -l:liboptbl.a

readbin: readbin.c
	gcc -o $@ $< $(CFLAGS)

install:
	@for bin in $(TARGETS) ; do \
		cp $$bin ~/opt/bin/ ; \
	done

clean:
	rm $(TARGETS)
