# Makefile for jpeg2ps
# (C) Thomas Merz 1994-2002
# Unsupported VMS support file for mms, initially provided by
# Rolf Niepraschk (niepraschk@ptb.de )

#
----------------------------------------------------------------------------
# VMS version
# Start with "mms /ignore=warn"
#
# throw out /DEFINE=A4 if you want letter format as default size

CC=cc
CCOPT= /DEFINE=A4/PREFIX=ALL
LDOPT=
OBJ=OBJ
EXE=.EXE
RM=rm

.c.$(OBJ) :
	$(CC) $(CCOPT) $*.c

all :	jpeg2ps$(EXE)
	@ !

jpeg2ps$(EXE) :	jpeg2ps.$(OBJ) readjpeg.$(OBJ) asc85ec.$(OBJ)
getopt.$(OBJ)
		LINK $(LDOPT) /EXE=$@ $+
		

clean :
        @ $ IF F$SEARCH("*.$(OBJ)",).NES."" THEN  DEL/LOG *.$(OBJ);*
        @ $ IF F$SEARCH("jpeg2ps$(EXE)",).NES."" THEN  DEL/LOG
jpeg2ps$(EXE);*

jpeg2ps.$(OBJ) :		jpeg2ps.c psimage.h

readjpeg.$(OBJ) :	readjpeg.c psimage.h

asc85ec.$(OBJ) :		asc85ec.c

getopt.$(OBJ) :		getopt.c
