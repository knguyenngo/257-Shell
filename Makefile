CC=gcc
CFLAGS= -c -g -Wall $(INCLUDES)

OBJECT_FILES= csapp.o shellex.o

all: sh257

sh257: $(OBJECT_FILES)
	$(CC) -pthread $(OBJECT_FILES) -o $@

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) $< -o $@

shellex.o: shellex.c csapp.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -v sh257 $(OBJECT_FILES)
