CC=$(PREFIX)gcc
STRIP=$(PREFIX)strip

SRC=FLATool.c flat.c fdx.c varsz.c util.c fs.c
OBJ=$(SRC:%.c=%.o)

EXE=FLATool

CFLAGS=-Wall -O2
LDFLAGS=-static

all : $(EXE)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE) : $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $(OBJ)
	$(STRIP) --strip-all $(EXE)

clean:
	-rm -f $(OBJ) $(EXE)
