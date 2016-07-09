CC      = /usr/bin/gcc
CFLAGS  = -Wall -g `pkg-config --cflags --libs elektra` -std=c99
LDFLAGS =
PROG    = main
OBJ     = main.o

all: $(PROG)

.PHONY: clean
clean:
	rm $(OBJ) $(PROG)

$(PROG): $(OBJ)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<
