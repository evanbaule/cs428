#include "packet.h"

void
paddr(unsigned char *a)
{
        printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

int main(int argc, char **argv)
{

    /*
    -------------------------------------------------
    Parse Command Line Arguments
    -------------------------------------------------
    */
    char* file_name = "";
    if(argc == 2)
        file_name = argv[1];
    else
        fprintf(stderr, "A file name argument is required.\n");
    
    char* hostname = "localhost"; //default
    int port = DEFAULT_PORT;
    if(argc == 3)
        hostname = argv[2]; //server hostname from execution params
    if(argc == 4)
        port = atoi(argv[3]); //server port from params

    /*
    -------------------------------------------------
    Configuring Client Side Addressing
    -------------------------------------------------
    */
    printf("Setting up client...\n");
    struct sockaddr_in myaddr;
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);

    /*
    -------------------------------------------------
    Opening socket
    -------------------------------------------------
    */
    int sfd;
    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failure creating socket\n");
        exit(EXIT_FAILURE);
    }

    /*
    -------------------------------------------------
    Binding to socket
    -------------------------------------------------
    */
    if(bind(sfd, (struct sockaddr*) &myaddr, sizeof(myaddr)) < 0)
    {
        perror("Failure binding socket.\n");
        exit(EXIT_FAILURE);
    }

    /*
    -------------------------------------------------
    Configuring Server Details
    -------------------------------------------------
    */
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(port);
    
    struct hostent *hp;
    hp = gethostbyname(hostname); //returns IP & server details
    if(!hp)
    {
        perror("Failure getting IP from given hostname.\n");
        exit(EXIT_FAILURE);
    }
    //Copy from hp result into serv_addr
    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);

    //Print IP Address of Server
    for (int i=0; hp->h_addr_list[i] != 0; i++)
    {
        paddr((unsigned char*) hp->h_addr_list[i]);
    }

    /*
    -------------------------------------------------
    Prepare file for transport
    -------------------------------------------------
    */
    FILE *in_file = fopen(file_name, "r");
    fseek(in_file, 0, SEEK_END); //seek to end of the file so we know how big it is
    int file_size = ftell(in_file);

    char* file_buffer = malloc(file_size + 1); // +1 for the sentinel 0 at the end
    fseek(in_file, 0, SEEK_SET); // reset file ptr to the beginning
    fread(file_buffer, 1, file_size, in_file); // read the file into the buffer
    file_buffer[file_size] = 0; // 0 terminate the string

    fclose(in_file); //close stream

    /*
    -------------------------------------------------
    Configure Metadata Packet
    -------------------------------------------------
    */
    int num_packets = file_size / PACKET_SIZE + 1;
    printf("num_packets:\t%d\n", num_packets);
    char* packets[num_packets];
    
    packet_meta *metadata = malloc(PACKET_SIZE);
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

    printf("Sending data: %s\n", meta_buff);
    int num_sent = 0;
    if((num_sent = sendto(sfd, (char*)&metadata, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("Return value from sendto():\t%d\n", num_sent);
        perror("Failed sending metadata to host");
        exit(EXIT_FAILURE);
    }
    printf("Sizeof data sent: %d\n", num_sent);

    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);

    // char* ack_buff = malloc(PACKET_SIZE);
    // int recvlen = recvfrom(sfd, ack_buff, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    // printf("Meta packet acknowledged:\n");
    // packet_ack* ack = (packet_ack*) ack_buff;
    // printf("----------------------\n");
    // printf("ACK summary:\n");
    // printf("\t- OP: %d\n", ack->op_code);
    // printf("\t- Packet #: %d\n", ack->packet_num);
    // printf("----------------------\n");

    printf("We sent this many bytes: %d\n", num_sent);
    close(sfd);
    return 0;
}
