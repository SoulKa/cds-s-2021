CFLAGS=-Ofast -Wall -std=c99
CLIBS=-lm -pthread
CC=gcc
RM=rm -f
EXEC=mandelbrot

all: $(EXEC)

$(EXEC):
	$(CC) $(CFLAGS) $(EXEC).c -o $(EXEC) $(CLIBS)

timing:
	$(CC) $(CFLAGS) -D MEASURE_TIMING $(EXEC).c -o $(EXEC) $(CLIBS)

run:
	cat judge.in | ./$(EXEC) >test.out 2>/dev/null

profile:
	$(CC) $(CFLAGS) -pg $(EXEC).c -o $(EXEC) $(CLIBS)

clean:
	$(RM) $(EXEC).o $(EXEC)
