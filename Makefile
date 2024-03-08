CC = gcc
CFLAGS = -Wall -Wextra -Wunused-parameter -Wunused-variable
LDFLAGS = -lpthread -lm

all: oss worker

oss: oss.o
	$(CC) $(CFLAGS) -o oss oss.o $(LDFLAGS)

worker: worker.o
	$(CC) $(CFLAGS) -o worker worker.o $(LDFLAGS)

oss.o: oss.c
	$(CC) $(CFLAGS) -c oss.c

worker.o: worker.c
	$(CC) $(CFLAGS) -c worker.c

clean:
	rm -f oss worker *.o
