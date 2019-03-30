
CC = gcc
CFLAGS = -g -Wall


HEADERS = sharedbuffer.h mypool.h encrypt.h
OBJECTS = sharedbuffer.o mypool.o encrypt.o

default: encrypt

%.o: %.c $(HEADERS)
    gcc -c $< -o $@

encrypt: $(OBJECTS)
     $(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
    -rm -f $(OBJECTS)
    -rm -f encrypt