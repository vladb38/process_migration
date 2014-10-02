
CC=gcc
INC=-I.. -I../@
FLAGS=-g -static

all: psave pload prog

psave: psave.c
	$(CC) $(INC) $(FLAGS) -o psave psave.c

pload: pload.c
	$(CC) $(INC) $(FLAGS) -o pload pload.c

prog: prog.c
	$(CC) $(INC) $(FLAGS) -o prog prog.c

clean:
	-rm psave pload prog *~ *.o