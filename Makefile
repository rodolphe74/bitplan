CC = m68k-atari-mint-gcc
LIBSTB=./stb


# standard gcc compilation
CFLAGS=-std=gnu99 -I$(LIBSTB) -I/usr/m68k-atari-mint/include/mint -O2
LINKFLAGS=-s -lc -lgcc -lm



all: main main30 bstrsz bstrsz30

main: wu
	$(CC) $(CFLAGS) screen.c wu.o -o screen.ttp $(LINKFLAGS)


main30: wu30
	$(CC) $(CFLAGS) -m68030 screen.c wu30.o -o screen30.ttp $(LINKFLAGS)


wu:
	$(CC) $(CFLAGS) -c wu.c $(LINKFLAGS)

wu30:
	$(CC) $(CFLAGS) -m68030 -c wu.c -o wu30.o $(LINKFLAGS)



bstrsz:
	$(CC) $(CFLAGS) bstrsz.c -o bstrsz.ttp $(LINKFLAGS)

bstrsz30:
	$(CC) $(CFLAGS) -m68030 bstrsz.c -o bstrsz30.ttp $(LINKFLAGS)


clean:
	rm *.o screen.ttp screen30.ttp bstrsz.ttp bstrsz30.ttp

format:
	clang-format --verbose -i *.c *.h
