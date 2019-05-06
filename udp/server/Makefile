#Evan M. Baule
#C428 - Computer Networks Project 2

all:	comp_all
comp_all: baule_server.c
	gcc baule_server.c -o server

clean:
	rm -f *.out *.o server

run: all
	./server
