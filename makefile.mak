
CC = gcc
CFLAGS = -g -Wall

OBJECTS = $(patsubst %.c, %.o, $(wildcard src/*.c))
HEADERS = $(wildcard inc/*.h)

default: encrypt

%.o: %.c $(HEADERS)
    gcc -c $< -o $@

encrypt: $(OBJECTS)
     $(CC) $(CFLAGS) $(OBJECTS) -o $@

clean:
    -rm -f $(OBJECTS)
    -rm -f encrypt