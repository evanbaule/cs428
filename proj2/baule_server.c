//Libraries
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h> //only used for printing out IP addresses during development
#include <ctype.h>

//My Headers
#include "packet.h"

int
main(int argc, char const ** argv)
{
    int serv_port = 8080;
    if(argc == 2)
    {
        serv_port = atoi(argv[1]);
    }

    printf("Starting server - listening on port: %d...\n", serv_port);

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
	socklen_t alen;//Not sure why the example uses this instead of sizeof
    int rqst; //file desc for the connected client
    
    //syscall create socket & get file descriptor
    int serv_sfd;
    if((serv_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Failure creating socket\n");
        exit(EXIT_FAILURE);
    }
    // printf("Server: socket_fd:\t%d\n", serv_sfd);

    //allow immediate reuse of the port
    int sockoptval = 1;
	setsockopt(serv_sfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    //Server configuration
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //0.0.0.0/ANY:PORT
    
    printf("Server port: %d\n", serv_addr.sin_port);
    printf("Server IP address is: %s\n", inet_ntoa( serv_addr.sin_addr));

    //Bind socket 
    if(bind(serv_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {   
        printf("Failure to bind\n");
        exit(EXIT_FAILURE);
    } 

    printf("Listening for connections...\n");
    if(listen(serv_sfd, 5) < 0) //allow up to 5 clients in queue
    {
        perror("Failure listening to socket.");
        exit(EXIT_FAILURE);
    }

    char* file_name = malloc(32*sizeof(char)); //idk if this is necessary i manage memory so poorly
    int file_size;
    int num_packets;
    char* file_buffer;

    while(1)
    {
        while((rqst = accept(serv_sfd, (struct sockaddr*) &client_addr, &alen)) < 0)
        {
            printf("Connection failed...");
            break;
        }
        printf("Connection established with user: %d\n", rqst);
        int brk = 1;
        while(brk)
        {
            char* packet_buffer = malloc(PACKET_SIZE);
            int num_read = read(rqst, packet_buffer, PACKET_SIZE);
            printf("WE READ THIS MANY BYTES:\t%d\n", num_read);
            
            packet_datagram* pckt = malloc(PACKET_SIZE);
            pckt = (packet_datagram*) packet_buffer;
            printf("Serialized pckt: %s\n", packet_buffer);
            printf("Recieved packet with OP code: %d\n", pckt->op_code + ' ');
            switch (pckt->op_code)
            {
                case 01: //Metadata Packet
                    printf("Recieved metadata packet... processing...\n");
                    packet_meta* metadata = malloc(PACKET_SIZE);
                    metadata = (packet_meta*) pckt;
                    printf("----------------------\n");
                    printf("Metadata summary:\n");
                    printf("\t- OP: %d\n", metadata->op_code);
                    printf("\t- File Name: %s\n", metadata->file_name);
                    printf("\t- File Size: %d\n", metadata->file_size);
                    printf("\t- # packets: %d\n", metadata->num_packets);
                    printf("----------------------\n");

                    file_name = metadata->file_name;
                    file_size = metadata->file_size;
                    num_packets = metadata->num_packets;

                    packet_ack* ack = malloc(PACKET_SIZE);
                    ack->op_code = 01;
                    ack->packet_num = 0;
                    char* ack_buff = malloc(PACKET_SIZE);
                    ack_buff = (char*) ack;
                    printf("Acknowledging metadata packet #: %d", ack->packet_num);
                    write(rqst, ack_buff, PACKET_SIZE);
                    break;
                case 02: //Datagram Packet
                    /* code */
                    printf("Received datagram packet... processing...\n");
                    printf("----------------------\n");
                    printf("Datagram summary:\n");
                    printf("\t- OP: %d\n", pckt->op_code);
                    printf("\t- Packet #: %d\n", pckt->packet_num);
                    printf("\t- Data:\n\t\t%s\n", pckt->data);
                    printf("----------------------\n");
                    break;
                case 03: //ACK Packet
                    /* code */
                    printf("ERROR: SERVER RECEIVED AN ACK FOR SOME REASON\n");
                    break;
                case 04: //Tail packet
                    /* code */
                    brk = 0;
                    break;
                default:
                    fprintf(stderr, "We should never have gotten to here\n");
                    brk = 0;
                    break;
            }

            // //RECIEVING
            // char rbuf[256];
            // read(rqst, rbuf, 256);
            // if(strcmp(rbuf, "exit\n") == 0)
            // {
            //     printf("User %d disconnected...\n", rqst);
            //     break;
            // }
            // else
            //     printf("From {%d}: %s", rqst, rbuf);

            // //SENDING
            // printf(">> ");
            // char wbuf[256]; //message buffer
            // fgets(wbuf, 256, stdin);

            // //wbuf2 is only used to check for exit message
            // char wbuf2[256]; //Make a copy of our message
            // memcpy(&wbuf2, wbuf, 256);
            // for(int i = 0; wbuf2[i]; i++){
            //     wbuf2[i] = tolower(wbuf2[i]); //convert message to lowercase for comparison
            // }
            // //if we enter exit, EXIT, Exit, etc. then we break the connection
            // if(strcmp(wbuf2, "exit\n") == 0) //include the '\n' char because fgets includes it in the buffer
            // {
            //     write(rqst, wbuf2, 256);
            //     printf("Closing connection...\n");
            //     break;
            // }
            // else
            // {
            //     //printf("Sending msg: %s\n", wbuf);
            //     write(rqst, wbuf, 256);
            // }
        }
    }

    //close socket before exiting process
    shutdown(serv_sfd, SHUT_RDWR);
    return 0;
}

