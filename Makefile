# Muscle Smartcard Development
# Makefile
# David Corcoran
# adapted by Kristian Beilke

CC       = cc
CFLAGS   = -g -fpic -w -Wall
LD       = ld
LEX      = flex
OBJ     := $(filter-out test.o, $(patsubst %.c,%.o,$(wildcard *.c)))
INCLUDE  = -I.

MAKEXE   = make
LIBNAME  = libgen_ifd_librfid.so
PREFIX   = /usr/local/pcsc
LIBS = -lrfid


DEFS     = -DPCSC_DEBUG=1 #-DATR_DEBUG=1

all: unix

clean:
	rm -f *.o $(LIBNAME) core

osx: $(OBJ)
	$(CC) -dynamiclib $(OBJ) -o $(LIBNAME)

unix: $(OBJ)
	$(LD) $(LIBS) -shared $(OBJ) -o $(LIBNAME)

$(patsubst %.c,%.o,$(wildcard *.c)) : %.o : %.c
	$(CC) $(CFLAGS) -c $< $(INCLUDE) $(DEFS)

