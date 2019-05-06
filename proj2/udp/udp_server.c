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

    int num_read = 0;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    
    char file_name[32];
    int file_size = 0;
    int num_packets = 0;
    FILE* out_file_ptr = fopen(file_name, "wb");

    packet* pckt = malloc(PACKET_SIZE); //packet buffer

    printf("Waiting on port %d\n", serv_port);
    int brk = 1;
    while(brk)
    {
        int ack_p_num = -1;
        num_read = recvfrom(server_sfd, pckt, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("Bytes Read:\t%d\n", num_read);
        pckt->op_code = ntohs(pckt->op_code);
        printf("Recieved packet with OP code: %d\n", pckt->op_code);

        switch (pckt->op_code)
        {
            case 01: //Metadata Packet
                printf("Recieved metadata packet... processing...\n");
                packet_meta* metadata = malloc(PACKET_SIZE);
                metadata = (packet_meta*) pckt;

                //convert to correct endian
                metadata->file_size = ntohl(metadata->file_size);
                metadata->num_packets = ntohl(metadata->num_packets);

                printf("----------------------\n");
                printf("Metadata summary:\n");
                printf("\t- OP: %d\n", metadata->op_code);
                printf("\t- File Name: %s\n", metadata->file_name);
                printf("\t- File Size: %d\n", metadata->file_size);
                printf("\t- # packets: %d\n", metadata->num_packets);
                printf("----------------------\n");

                memcpy(file_name, metadata->file_name, 32);
                file_size = metadata->file_size;
                num_packets = metadata->num_packets;

                // packet_ack* ack = malloc(PACKET_SIZE);
                // ack->op_code = 01;
                // ack->packet_num = 0;
                // char* ack_buff = malloc(PACKET_SIZE);
                // ack_buff = (char*) ack;
                // printf("Acknowledging metadata packet #: %d", ack->packet_num);
                // write(rqst, ack_buff, PACKET_SIZE);
                ack_p_num = 0;
                break;
            case 02: //Datagram Packet
                printf("Received datagram packet... processing...\n");
                packet_datagram *dg = malloc(PACKET_SIZE);

                dg = (packet_datagram*) pckt;

                //convert endianness
                dg->packet_num = ntohl(dg->packet_num);
                ack_p_num = dg->packet_num;

                printf("----------------------\n");
                printf("Datagram summary:\n");
                printf("\t- OP: %d\n", dg->op_code);
                printf("\t- Packet #: %d\n", dg->packet_num);
                //printf("\t- Data:\n\t\t%s\n", dg->data);
                printf("----------------------\n");
                //append to file
                fwrite(dg->data, sizeof(char), sizeof(dg->data), out_file_ptr); 
                
                break;
            case 03: //ACK Packet
                printf("ERROR: SERVER RECEIVED AN ACK FOR SOME REASON\n");
                exit(EXIT_FAILURE);
                break;
            case 04: //Tail packet
                brk = 0;
                break;
            default:
                fprintf(stderr, "We should never have gotten to here\n");
                brk = 0;
                break;
        }

        packet_ack *ack = malloc(PACKET_SIZE);
        ack->op_code = htons(3);
        ack->packet_num = htonl(ack_p_num);
        sendto(server_sfd, ack, sizeof(ack), 0, (struct sockaddr *)&remaddr, addrlen);
    }

    close(server_sfd);
    return 0;
}
