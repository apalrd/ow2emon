PROG = ow2emon
SOURCES = $(PROG).c
CFLAGS = -O -W -Wall -std=c99 $(CFLAGS_EXTRA)

all: $(SOURCES)
	$(CC) -o $(PROG) $(SOURCES) $(CFLAGS) -lowcapi -I. -I/opt/owfs/include -L/opt/owfs/lib

clean:
	rm -rf *.gc* *.dSYM *.exe *.obj *.o a.out $(PROG)
