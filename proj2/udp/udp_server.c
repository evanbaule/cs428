#include "packet.h"

int main(int argc, char **argv)
{
    int serv_port = DEFAULT_PORT;
    if(argc == 2)
    {
        serv_port = atoi(argv[1]);
    }

    int server_sfd;
    if((server_sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failed opening socket on server.\n");
        exit(EXIT_FAILURE);
    }

    //Server configuration
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(server_sfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Failure binding socket.\n");
        exit(EXIT_FAILURE);
    }



    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    
    int num_read = 0;
    char* packet_buffer = malloc(PACKET_SIZE);
    packet_meta* metadata = malloc(PACKET_SIZE);

    printf("Waiting on port %d\n", serv_port);
    int brk = 1;
    while(brk)
    {
        
        num_read = recvfrom(server_sfd, (char*)&metadata, sizeof(metadata), 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("Bytes Read:\t%d\n", num_read);
        
        packet_datagram* pckt = malloc(PACKET_SIZE);
        pckt = (packet_datagram*) packet_buffer;
        printf("Serialized packet: %s\n", packet_buffer);
        printf("Recieved packet with OP code: %d\n", pckt->op_code + ' ');
        brk = 0;
    }

    close(server_sfd);
    return 0;
}
