CC = m68k-atari-mint-gcc
# LIBCMINI=./libcmini-0.54
LIBCMINI=./libcmini-fast
LIBMATH=./fdlibm
LIBSTB=./stb
OBJMATH=./fdlibm/68000.soft-float


# Compilation using libcmini can't use math & stb - work with picojpeg
# CFLAGS=-std=gnu99 -I$(LIBCMINI)/include -nostdlib $(LIBCMINI)/lib/crt0.o
# LINKFLAGS=-s -L$(LIBCMINI)/lib -lcmini -lgcc 

# try to use math lib libcmini
# CFLAGS=-std=gnu99 -O3 -I$(LIBSTB) -I$(LIBCMINI)/include -I$(LIBMATH)/include  -nostdlib $(LIBCMINI)/lib/crt0.o
# LINKFLAGS=-s -L$(LIBCMINI)/lib -lcmini -lgcc 

# standard gcc compilation
CFLAGS=-std=gnu99 -I$(LIBSTB) -I/usr/m68k-atari-mint/include/mint -O2
LINKFLAGS=-s -lc -lgcc -lm



all: main bestrsz

main: wu
	$(CC) $(CFLAGS) screen.c wu.o -o screen.ttp $(LINKFLAGS)


main30:
	$(CC) $(CFLAGS) -m68030 screen.c -o screen30.ttp $(LINKFLAGS)


wu:
	$(CC) $(CFLAGS) -c wu.c $(LINKFLAGS)


bestrsz:
	$(CC) $(CFLAGS) bestrsz.c -o bestrsz.ttp $(LINKFLAGS)

# main: picojpeg.o 
# 	$(CC) $(CFLAGS) screen.c -o screen.tos picojpeg.o \
# 		$(OBJMATH)/s_issigf.o \
# 	   	$(OBJMATH)/s_roundf.o \
# 		$(OBJMATH)/e_powf.o \
# 		$(OBJMATH)/s_isinf.o \
# 		$(OBJMATH)/e_sqrtf.o \
# 	   	$(OBJMATH)/s_isinff.o \
# 		$(OBJMATH)/s_scalbnf.o \
# 		$(OBJMATH)/e_log.o \
# 		$(OBJMATH)/s_frexp.o \
# 		$(OBJMATH)/e_pow.o \
# 		$(OBJMATH)/s_issig.o \
# 		$(OBJMATH)/e_sqrt.o \
# 		$(OBJMATH)/s_scalbn.o \
# 		$(OBJMATH)/s_ldexp.o \
# 		$(OBJMATH)/s_finite.o \
# 	   	$(LINKFLAGS)

# assembly compilation
# putpixel:
# 	$(CC) -c putpixel.s -Wa,-S


clean:
	rm *.o screen.ttp screen30.ttp

format:
	clang-format --verbose -i *.c *.h
