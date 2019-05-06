#Evan M. Baule
#C428 - Computer Networks Project 2

all:	comp_all
comp_all: baule_client.c
	gcc baule_client.c -o client

clean:
	rm -f *.out *.o client

run: all
	./client bees.txt
