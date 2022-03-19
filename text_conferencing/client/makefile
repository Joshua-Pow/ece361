OBJS	= client.o
SOURCE	= client.c
HEADER	= ../conferencing.h
OUT	= client
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

client.o: client.c
	$(CC) $(FLAGS) client.c 


clean:
	rm -f $(OBJS) $(OUT)