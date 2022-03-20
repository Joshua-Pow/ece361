OBJS	= server.o
SOURCE	= server.c
HEADER	= ../conferencing.h
OUT	= server
CC	 = gcc
FLAGS	 = -g -c
LFLAGS	 = -lm

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

server.o: server.c
	$(CC) $(FLAGS) server.c 


clean:
	rm -f $(OBJS) $(OUT)