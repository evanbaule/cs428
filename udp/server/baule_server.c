#include "../packet.h"

int main(int argc, char **argv)
{
    /*
    -------------------------------------------------
    Parse Command Line Arguments
    -------------------------------------------------
    */
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

    /*
    -------------------------------------------------
    Configure Server Details
    -------------------------------------------------
    */
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

    /*
    -------------------------------------------------
    Initialize File Details
    -------------------------------------------------
    */
    int num_read = 0;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    
    char file_name[32];
    int file_size = 0;
    int file_size_copy = 0;
    int num_packets = 0;
	int total_written = 0;
    FILE* out_file_ptr;

    packet* pckt = malloc(PACKET_SIZE); //packet buffer
	packet_ack *ack = malloc(PACKET_SIZE); //so that we only malloc once

    printf("Listening on port %d\n", serv_port);
    int brk = 1;
    while(brk)
    {
        int ack_p_num = -1; //default to tail packet

        /*
        -------------------------------------------------
        Read the next packet and route based on op_code
        -------------------------------------------------
        */
        num_read = recvfrom(server_sfd, pckt, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        pckt->op_code = ntohs(pckt->op_code); //read the opcode to decide how to process the packet we just got

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
                out_file_ptr = fopen(file_name, "wb");
                if(!out_file_ptr)
                {
                    perror("FAILED OPENING FILE");
                    exit(EXIT_FAILURE);
                }

                file_size = metadata->file_size;
                file_size_copy = file_size;
                num_packets = metadata->num_packets;

				//Establish ack for META
                ack_p_num = 0;

				//free(metadata); //dont need anymore

                break;
            case 02: //Datagram Packet
                printf("Received datagram packet... processing...\n");
                packet_datagram *dg = malloc(PACKET_SIZE);

                dg = (packet_datagram*) pckt;

                //convert endianness
                dg->packet_num = ntohl(dg->packet_num);
                ack_p_num = dg->packet_num; //establish ack for dg

                printf("----------------------\n");
                printf("Datagram summary:\n");
                printf("\t- OP: %d\n", dg->op_code);
                printf("\t- Packet #: %d\n", dg->packet_num);
                //printf("\t- Data:\n\t\t%s\n", dg->data);
                printf("----------------------\n");
				
                //append to file
                if(file_size_copy < 1494) // if we have less than a full data block to write, only write the remainder not 1494
                {
					//only write file data - no trailing 0s
                    total_written += fwrite(dg->data, sizeof(char), file_size_copy, out_file_ptr);
                }
                else
                {
                    total_written += fwrite(dg->data, sizeof(char), sizeof(dg->data), out_file_ptr);
                    file_size_copy -= sizeof(dg->data); //this helps us only write the correct amt for the 
                }	
				
				//free(dg); //free datagram after write

                break;
            case 03: //ACK Packet
                printf("ERROR: SERVER RECEIVED AN ACK FOR SOME REASON\n");
                exit(EXIT_FAILURE);
                break;
            case 04: //Tail packet
                brk = 0; //set brk so that we break out of the inf loop and stop listening
                break;
            default:
                fprintf(stderr, "We should never have gotten to here\n"); // <-- true
                brk = 0;
                break;
        }

        /*
        -------------------------------------------------
        Acknowledge The Packet We Just Got
        -------------------------------------------------
        */
        
        ack->op_code = htons(3);
        ack->packet_num = htonl(ack_p_num);
        sendto(server_sfd, ack, sizeof(ack), 0, (struct sockaddr *)&remaddr, addrlen);

		
    }
	
	printf("Finished recieving: %s, total %d bytes written.\n", file_name, total_written);

    /*
    -------------------------------------------------
    Shutdown
    -------------------------------------------------
    */
	
	free(pckt);
	free(ack);

    fclose(out_file_ptr);
    close(server_sfd);
    return 0;
}
