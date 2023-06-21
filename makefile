CCC	:= gcc

CFLAGS := -I/usr/include/x86_64-linux-gnu -I/usr/include
LDFLAGS := -lspandsp -lsndfile

SRC    := g726.c

all:
	$(CC) -o g726 $(CFLAGS) $(SRC) $(LDFLAGS)

clean:
	rm *.o
	rm g726