CC=$(PREFIX)gcc
STRIP=$(PREFIX)strip

include Makefile.inc

SRC=FLATool.c flat.c fdx.c varsz.c util.c fs.c
OBJ=$(SRC:%.c=%.o)

EXE=$(call exe-name,FLATool)

CFLAGS +=-Wall -O2

ifneq "$(OS)" "apple"
  LDFLAGS +=-static
  STRIP_FLAGS :=--strip-all
else
  STRIP_FLAGS :=
endif

all : $(EXE)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE) : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)
	$(STRIP) $(STRIP_FLAGS) $(EXE)

clean:
	-rm -f $(OBJ) $(EXE)
