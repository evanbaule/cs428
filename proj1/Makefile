#Evan M. Baule
#C428 - Computer Networks

all:	comp_all

comp_all: server.c client.c
	gcc server.c -o server
	gcc client.c -o client

clean:
	rm -f *.o client server

run: all
	./prog4swap

	