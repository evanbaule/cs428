#Evan M. Baule
#C428 - Computer Networks Project 2

all:	comp_all
comp_all: udp_server.c udp_client.c
	gcc udp_server.c -o server
	gcc udp_client.c -o client

clean:
	rm -f *.out *.o client server

runc: all
	./client ../samurai.txt
