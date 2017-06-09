CC:=gcc

all: main.o
	$(CC) $(CFLAGS) main.o -o rax

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

clean:
	$(RM) rax main.o

install: all
	mv rax /usr/local/bin/rax
