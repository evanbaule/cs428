#Evan M. Baule
#C428 - Computer Networks Project 2

all:	comp_all
comp_all: baule_server.c baule_client.c
	gcc baule_server.c -o baule_server
	gcc baule_client.c -o baule_client

clean:
	rm -f *.o baule_client baule_server

run: all
	./prog4swap

	