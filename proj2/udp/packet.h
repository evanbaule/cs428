#ifndef PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#define PACKET_SIZE 1500;
#define DEFAULT_HOST "localhost"
#define DEFAULT_PORT 8080

// typedef struct packet
// {
//     short op_code; //00
// } __attribute__((packed)) packet;

typedef struct packet_meta
{
    short op_code; //01
    char file_name[32]; //max length set to ensure fixed size, tells name of inbound file
    int file_size;  //tells size of inbound file
    int num_packets; // total # of packets for the entire file (might as well send it with all this space)
} __attribute__ ((packed)) packet_meta;

typedef struct packet_datagram
{
    short op_code; //02
    int packet_num;
    char data[1494]; //1500 - 6 for the op code and packet number
} __attribute__((packed)) packet_datagram;

typedef struct packet_ack
{
    short op_code; //03
    int packet_num;
} __attribute__((packed)) packet_ack;

typedef struct packet_tail
{
    short op_code; //04
} __attribute((packed)) packet_tail;

#endif // !PACKET_H