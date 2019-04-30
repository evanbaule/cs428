// Libraries
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h> //only used for printing out IP addresses during development
#include <ctype.h>

// My Headers
#include "packet.h"

int
main(int argc, char ** argv)
{
    char* file_name = "";
    if(argc == 2)
        file_name = argv[1];
    else
        fprintf(stderr, "A file name argument is required.\n");
    
    char* hostname = "localhost"; //default
    int port = 0;
    if(argc == 3)
        hostname = argv[2]; //server hostname from execution params
    if(argc == 4)
        port = atoi(argv[3]);

    printf("Setting up client...\n");

    //syscall create socket TCP/IPv4
    int client_sfd = socket(AF_INET, SOCK_STREAM, 0); assert(client_sfd != -1 && "Socket creation syscall failure.");
    //printf("socket_fd:\t%d\n", client_sfd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");
   
    struct sockaddr_in client_addr;
    memset((char *)&client_addr, 0, sizeof(client_addr)); //zero out memory for safety
    client_addr.sin_family = AF_INET; //IPv4
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); //client address doesn't really matter
    client_addr.sin_port = htons(0); //client port doesn't really matter either

    int brv = bind(client_sfd, (struct sockaddr *)&client_addr, sizeof(client_addr)); //assert(brv != -1 && "Failure binding to port: 8080");
    if(brv == -1)
    { 
        printf("brv@fail:\t%d\n", brv);
        perror("Failure binding socket to port");
        exit(EXIT_FAILURE);
    }
    //printf("brv:\t%d\n", brv);

    struct hostent *hp;
    hp = gethostbyname(hostname); //why is this a warning?
    if (!hp) {
        fprintf(stderr, "could not obtain address of %s\n", hostname);
        return 0;
    }

    //Configure server details
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    //Copy hostname ip addr into the server struct
    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    // printf("ip post change:\t%s\n", inet_ntoa(serv_addr.sin_addr));
    // printf("port:\t%d\n", serv_addr.sin_port);

    //serv_addr.sin_addr.s_addr = inet_addr(hostname);

    printf("Attempting to establish connection to: %s:%d...\n", hostname, port);
    int connect_r = 0;
    if((connect_r = connect(client_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) 
    {
        perror("Failure connecting to server");
        exit(EXIT_FAILURE);
    }
    
    FILE *in_file = fopen(file_name, "r");
    fseek(in_file, 0, SEEK_END); //seek to end of the file so we know how big it is
    int file_size = ftell(in_file);

    char* file_buffer = malloc(file_size + 1); // +1 for the sentinel 0 at the end
    fseek(in_file, 0, SEEK_SET); // reset file ptr to the beginning
    fread(file_buffer, 1, file_size, in_file); // read the file into the buffer
    file_buffer[file_size] = 0; // 0 terminate the string

    fclose(in_file); //close stream

    // printf("----------------------\n");
    // printf("%s\n", file_buffer);
    // printf("----------------------\n");

    int num_packets = file_size / PACKET_SIZE + 1;
    printf("num_packets:\t%d\n", num_packets);
    char* packets[num_packets];
    
    packet_meta* metadata = malloc(PACKET_SIZE);
    metadata->op_code = 01;
    memcpy(metadata->file_name, &file_name, 32);
    metadata->file_size = file_size;
    metadata->num_packets = num_packets;
    printf("----------------------\n");
    printf("Metadata summary:\n");
    printf("\t- OP: %d\n", metadata->op_code);
    printf("\t- File Name: %s\n", metadata->file_name);
    printf("\t- File Size: %d\n", metadata->file_size);
    printf("\t- # packets: %d\n", metadata->num_packets);
    printf("----------------------\n");
    

    char* meta_buff = malloc(PACKET_SIZE);
    meta_buff = (char*) metadata;
    printf("Dispatching metadata, waiting for response...\n");
    write(client_sfd, meta_buff, PACKET_SIZE);

    //Maybe make this a function since we use it twice
    char* ack_buff = malloc(PACKET_SIZE);
    read(client_sfd, ack_buff, PACKET_SIZE);
    printf("Meta packet acknowledged:\n");
    packet_ack* ack = (packet_ack*) ack_buff;
    printf("----------------------\n");
    printf("ACK summary:\n");
    printf("\t- OP: %d\n", ack->op_code);
    printf("\t- Packet #: %d\n", ack->packet_num);
    printf("----------------------\n");

    if(ack->op_code != 3)
    {
        fprintf(stderr, "Meta ACK Packet had the wrong OP code: %d\n", ack->op_code);
    }
    if(ack->packet_num != 0)
    {
        fprintf(stderr, "Meta ACK Packet had the wrong packet number: %d\n", ack->packet_num);
    }


    int i;
    for(i = 0; i < num_packets; i++)
    {
        //char buff[1494];
        //memcpy(&buff, &file_buffer[i*1500], 1500);
        //printf("packet #: %d\n\t%s\n", i, buff);
        printf("Packetizing data...\n");
        packet_datagram* dg = malloc(PACKET_SIZE);
        dg->op_code = 02;
        dg->packet_num = i+1;
        memcpy(dg->data, &file_buffer[i*PACKET_SIZE], PACKET_SIZE);
        printf("----------------------\n");
        printf("Datagram summary:\n");
        printf("\t- OP: %d\n", dg->op_code);
        printf("\t- Packet #: %d\n", dg->packet_num);
        printf("\t- Data:\n\t\t%s\n", dg->data);
        printf("----------------------\n");
        //Now we memcpy dg into the socket and send it along, and we reassemble the file on the other side :)
        //write(client_fd, (char*(maybe??))dg, 1500 //sizeof(dg));
        //block and wait for ack
        //read(client_fd, ack_buffer, 1500//sizeof(ack));
        //packet_ack ack = (packet_ack) ack_buffer //cast onto the byte array
        //check ack -> op code, ack -> packet # matches the one we sent out

        char* packet_buff = malloc(PACKET_SIZE);
        packet_buff = (char*) dg;
        printf("Serialized datagram: %s\n", packet_buff);
        printf("Dispatching datagram...\n");
        write(client_sfd, packet_buff, PACKET_SIZE);

        read(client_sfd, ack_buff, PACKET_SIZE);
        printf("Datagram acknowledged...\n");
        ack = (packet_ack*) ack_buff;
        printf("----------------------\n");
        printf("ACK summary:\n");
        printf("\t- OP: %d\n", ack->op_code);
        printf("\t- Packet #: %d\n", ack->packet_num);
        printf("----------------------\n");

        if(ack->op_code != 3)
        {
            fprintf(stderr, "ACK Packet %d had the wrong OP code: %d\n", i+1, ack->op_code);
        }
        if(ack->packet_num != i+1)
        {
            fprintf(stderr, "ACK Packet # %d had the wrong packet number: %d\n", i+1, ack->packet_num);
        }
    }

    packet_tail* tail = malloc(PACKET_SIZE);
    tail->op_code = 04;
    printf("----------------------\n");
    printf("Tail summary:\n");
    printf("\t- OP: %d\n", tail->op_code);
    printf("----------------------\n");
    char* tail_buff = malloc(PACKET_SIZE);
    tail_buff = (char*) tail;
    printf("Dispatching EOF tail packet, waiting for response\n");
    write(client_sfd, tail_buff, PACKET_SIZE);

    //Maybe make this a function since we use it twice
    read(client_sfd, ack_buff, PACKET_SIZE);
    printf("Tail packet acknowledged:\n");
    ack = (packet_ack*) ack_buff;
    printf("----------------------\n");
    printf("ACK summary:\n");
    printf("\t- OP: %d\n", ack->op_code);
    printf("\t- Packet #: %d\n", ack->packet_num);
    printf("----------------------\n");

    if(ack->op_code != 3)
    {
        fprintf(stderr, "Tail ACK Packet had the wrong OP code: %d\n", ack->op_code);
    }
    if(ack->packet_num != i)
    {
        fprintf(stderr, "Tail ACK Packet had the wrong packet number: %d\n", ack->packet_num);
    }

    printf("Hopefully we made it here without anything breaking...\n");


    // //Sending data
    // while(1)
    // {
    //     //SENDING
    //     // printf(">> ");
    //     // char wbuf[256]; //message buffer
    //     // fgets(wbuf, 256, stdin);

    //     //wbuf2 is only used for checking for exit messages
    //     //char wbuf2[256]; //Make a copy of our message
    //     //char packet_buffer[1500];
    //     //memcpy(&packet_buffer, )
    //     memcpy(&wbuf2, wbuf, 256);
    //     for(int i = 0; wbuf2[i]; i++){
    //         wbuf2[i] = tolower(wbuf2[i]); //convert message to lowercase for comparison
    //     }
    //     //if we enter exit, EXIT, Exit, etc. then we break the connection
    //     if(strcmp(wbuf2, "exit\n") == 0) //include the '\n' char because fgets includes it in the buffer
    //     {
    //         write(client_sfd, wbuf2, 256);
    //         printf("Closing connection...\n");
    //         break;
    //     }
    //     else
    //     {
    //         //printf("Sending msg: %s\n", wbuf);
    //         write(client_sfd, wbuf, 256);
    //     }

    //     //RECIEVING
    //     char rbuf[256];
    //     read(client_sfd, rbuf, 256); //blocking read, waits for a response
    //     if(strcmp(rbuf, "exit\n") == 0)
    //     {
    //         printf("Server disconnected...\n");
    //         break;
    //     }
    //     else
    //         printf("From {Server}: %s", rbuf);
    // }

    //Closing socket before shutting down
    shutdown(client_sfd, SHUT_RDWR);
    return 0;
}