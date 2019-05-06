# CS428 Project 2 - UDP File Transfer
# Evan Baule

Resources:
	https://www.cs.rutgers.edu/~pxk/417/notes/sockets/udp.html
	Lots of Stack Overflow threads

Compiling & Running: 
    Folder Structure
    .
    ├── README.txt
    ├── client/
    │   ├── Makefile
    │   ├── baule_client.c
    ├── packet.h
    └── server/
        ├── Makefile
        └── baule_server.c

        The client and server folders are isolated so that when I send a file from client/ it appears in server/
        To compile each of the source code files, run 'make' in the respective folder.
            - 'make'
        To run the server:
            - 'make run' // runs the server on port 8080
            - './server <port>' // runs the server on a specified port
        To run the client: (make sure the server is running before running the client)
            - './client <file_name.ext> <hostname> <port #>

Completion Status:
    - This version has been somewhat rigorously tested to match the test cases on the spec: 
        - Test cases successful:
            - Sending small & large (30M+) text files to different host on same network (in g7)
            - Sending small (5M) & large (30M) mp4 video files
    - Each packet displays a summary prior to being sent from CLIENT -->(packet)--> SERVER

Summary of Program Architecture:
    - Server opens and starts listening on <port>
    - Client opens socket & configures host details for <hostname>:<port>
    - Client prepares file metadata for transfer
        - Sends metadata packet & waits for it to be acknowledged
    - Server receives metadata & opens "wb" file stream
        - Server then acknowledges the metadata packet
    - Client receives ACK for metadata and begins sending datagram packets (1494B worth of actual file data) to the server
        - Client stores all packets for the file on the heap at runtime (fseek(SEEK_END) gets us the size of the file, so we know how much to malloc)
        - Client blocking waits for the server to send an ACK with appropriate packet # for each packet sent (Stop & Wait ARQ)
    - Server listens for new packets & writes 1494 bytes to the file stream each time it receives a datagram, then ACKs that packet.
    - Client finishes sending datagram packets & builds a tail packet to signal to the server that the transmission is complete.
    - Server gets the tail packet, closes the file stream and exits (this could be modifies to accept multiple files but as of right now it only accepts 1).

Packet structure:
    - Metadata
        // CLIENT -- > (Meta) -- > SERVER
        - op code (short - 2B)
			- META OP: 1
        - file name (char[32] - 32B)
        - file size (int - 4B)
        - number of packets (int - 4B)
         _____________________________________________(1500)__________________________________________________
        | OP  |   file_name     |  file_size   | # packets  |                    Empty                        |
        |_(2)_|______(32)_______|_____(4)______|_____(4)____|___________________(1456)________________________|

    - Datagram
        // CLIENT -- > (Datagram) -- > SERVER contains 1494 bytes of file data
        - op code (short - 2B) 
			- DG OP : 2
        - packet number (int - 4B)
        - data (char[1494] - 1494B)
         _____________________________________________(1500)__________________________________________________
        | OP  | packet #   |                    Data                                                          |
        |_(2)_|____(4)_____|___________________(1494)_________________________________________________________|
    
    - ACK
        // SERVER -- > (ACK) -- > CLIENT to acknowledge packet w packet number ack->packet_num
        - op code (short - 2B)
			- ACK OP : 3
        - packet number(int - 4B)
         _____________________________________________(1500)__________________________________________________
        | OP  | packet #   |                    Empty                                                         |
        |_(2)_|____(4)_____|____________________(1494)________________________________________________________|
        
    - Tail
        // CLIENT -- > (Tail) -- > SERVER
        // Used to signify the end of the transmission - tells the server that the file is done writing
        - op code (short - 2B)
			- TAIL OP: 4
         _____________________________________________(1500)__________________________________________________
        | OP  |                                 Empty                                                         |
        |_(2)_|_________________________________(1498)________________________________________________________|








