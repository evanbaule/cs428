Evan M. Baule
CS428 - Computer Networks
Project 1 - IRC Socket

Usage:
    - Compiling
        - make
            - runs Makefile macro to compile both files
        - (or)
            gcc client.c -o client
            gcc server.c -o server
    - Running
        ./server [port]
            - starts server with optional port # defined, defaults to port :8080
        ./client [hostname] [port]
            - start client with specified hostname (default 'localhost') and specified port (default :8080 as well)
            - WARNING: there currently is no support for passing an IPv4 formatted adress as a command argument, only a domain name (I'm using gethostbyname(name) to get the hostent)
                - in order to do this, you would have to manually set the IP address of serv_addr (in client) using inet_addr

Implementation details:
    - default host: 'localhost',
    - default port: 8080,
    - client queue size (backlog param for listen()): 5,
    - max message size: 256,
    - send 'exit' from either endpoint (when it is their turn) to close the connection

When the application starts, a connection is established between the client & server. Then, the server waits for the client to send a message (unfortunately the client must send the first message due to the details of the implementation). After that, the client and server take turns sending messages back and forth, one message at a time. The '>>' prompt indicates the current active user.
