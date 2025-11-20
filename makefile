CC = gcc
CFLAGS = -g -O0 -Wall -Wextra -std=c11 -m32

# Default target - build all executables
all: myshell looper mypipe

# Build myshell (requires LineParser.c)
myshell: myshell.c LineParser.c
	$(CC) $(CFLAGS) -o myshell myshell.c LineParser.c

# Build Looper separately
looper: Looper.c
	$(CC) $(CFLAGS) -o looper Looper.c

# Build mypipe
mypipe: mypipe.c
	$(CC) $(CFLAGS) -o mypipe mypipe.c

# Build LineParser test separately (if needed)
lineparser: LineParser.c
	$(CC) $(CFLAGS) -c LineParser.c -o LineParser.o

# Clean all build artifacts
clean:
	rm -f *.o myshell looper mypipe

# Phony targets (not actual files)
.PHONY: all clean myshell looper mypipe lineparser
