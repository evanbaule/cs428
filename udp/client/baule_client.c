#include "../packet.h"

int main(int argc, char **argv)
{

    /*
    -------------------------------------------------
    Parse Command Line Arguments
    -------------------------------------------------
    */
    char* file_name = "";
    file_name = argv[1];
    
    char* hostname = "localhost"; //default
    int port = DEFAULT_PORT;
    if(argc == 3)
        hostname = argv[2]; //server hostname from execution params
    if(argc == 4)
        port = atoi(argv[3]); //server port from params

	printf("host: %s\n", hostname);
	
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
    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);

    /*
    -------------------------------------------------
    Prepare file for transport
    -------------------------------------------------
    */
    FILE *in_file = fopen(file_name, "rb");
	if(!in_file)
		exit(EXIT_FAILURE);
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
    int num_packets = (file_size / 1494) + 1;
    printf("num_packets:\t%d\n", num_packets);
    char* packets = malloc(num_packets * 1500);
    
    packet_meta* metadata = malloc(PACKET_SIZE);
    memset( metadata->empty, 0, sizeof(metadata->empty) );

    metadata->op_code = 01;
    memcpy(metadata->file_name, file_name, 32);
    metadata->file_size = file_size;
    metadata->num_packets = num_packets;
    printf("----------------------\n");
    printf("Metadata summary:\n");
    printf("\t- OP: %d\n", metadata->op_code);
    printf("\t- File Name: %s\n", metadata->file_name);
    printf("\t- File Size: %d\n", metadata->file_size);
    printf("\t- # packets: %d\n", metadata->num_packets);
    printf("----------------------\n");

    metadata->op_code = htons(metadata->op_code);
    metadata->file_size = htonl(metadata->file_size);
    metadata->num_packets = htonl(metadata->num_packets);

    // char* meta_buff = malloc(PACKET_SIZE);
    // meta_buff = (char*) metadata;
    printf("Dispatching metadata, waiting for response...\n");

    //printf("Sending data: %s\n", meta_buff);
    int num_sent = 0;
    if((num_sent = sendto(sfd, metadata, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("Return value from sendto():\t%d\n", num_sent);
        perror("Failed sending metadata to host");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remaddr; //not used?
    socklen_t addrlen = sizeof(remaddr);

    packet_ack* ack = malloc(PACKET_SIZE);
    memset( ack->empty, 0, sizeof(ack->empty) );
    int recvlen = recvfrom(sfd, ack, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
    
    ack->op_code = ntohs(ack->op_code);
    ack->packet_num = ntohl(ack->packet_num);

    /*
    -------------------------------------------------
    Acknowdledging Metadata Packet
    -------------------------------------------------
    */
    printf("Meta packet acknowledged:\n");
    printf("----------------------\n");
    printf("ACK summary:\n");
    printf("\t- OP: %d\n", ack->op_code);
    printf("\t- Packet #: %d\n", ack->packet_num);
    printf("----------------------\n");
	free(metadata);

    int i;
    for(i = 0; i < num_packets; i++)
    {
    	/*
    	-------------------------------------------------
    	Configuring Datagram Packet
    	-------------------------------------------------
    	*/	
        //printf("PROCESSING PACKET: %d\n", i);
        char buff[1494];
        memcpy(&buff, &file_buffer[i*1494], 1494);
        //printf("packet #: %d\n\t%s\n", i, buff);
        //printf("Packetizing data...\n");
        packet_datagram* dg = malloc(PACKET_SIZE);
        dg->op_code = 02;
        dg->packet_num = i+1;
        memcpy(dg->data, &file_buffer[i*1494], 1494);

        printf("----------------------\n");
        printf("Datagram summary:\n");
        printf("\t- OP: %d\n", dg->op_code);
        printf("\t- Packet #: %d\n", dg->packet_num);
        //printf("\t- Data:\n\t\t%s\n", dg->data);
        printf("----------------------\n");
        printf("Dispatching datagram...\n");

        dg->op_code = htons(dg->op_code);
        dg->packet_num = htonl(dg->packet_num);

        if((num_sent = sendto(sfd, (char*)dg, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
        {
            printf("Return value from sendto():\t%d\n", num_sent);
            perror("Failed sending datagram to host");
            exit(EXIT_FAILURE);
        }

		/*
    	-------------------------------------------------
    	Acknowledging Datagram Packet
    	-------------------------------------------------
    	*/	
		int recvlen = recvfrom(sfd, ack, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		ack->op_code = ntohs(ack->op_code);
		ack->packet_num = ntohl(ack->packet_num);
        printf("Datagram acknowledged...\n");
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

		free(dg); //getting rid of the dg space
    }

	/*
	-------------------------------------------------
	Configuring Tail Packet
	-------------------------------------------------
	*/	
    packet_tail* tail = malloc(PACKET_SIZE);
    tail->op_code = 04;
    memset( tail->empty, 0, sizeof(tail->empty) );
    printf("----------------------\n");
    printf("Tail summary:\n");
    printf("\t- OP: %d\n", tail->op_code);
    printf("----------------------\n");
    printf("Dispatching EOF tail packet, waiting for response\n");

    tail->op_code = htons(tail->op_code); //convert endianness

    if((num_sent = sendto(sfd, (char*)tail, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
    {
        printf("Return value from sendto():\t%d\n", num_sent);
        perror("Failed sending metadata to host");
        exit(EXIT_FAILURE);
    }

	free(tail);


	/*
	-------------------------------------------------
	Acknowledging Tail Packet
	-------------------------------------------------
	*/	
	recvlen = recvfrom(sfd, ack, PACKET_SIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
	ack->op_code = ntohs(ack->op_code); //convert endianness
	ack->packet_num = ntohl(ack->packet_num); //convert endianness
    printf("Tail acknowledged...\n");
	printf("----------------------\n");
	printf("ACK summary:\n");
    printf("\t- OP: %d\n", ack->op_code);
    printf("\t- Packet #: %d (tail should be -1)\n", ack->packet_num);
    printf("----------------------\n");

	//confirm correct ACK format
    if(ack->op_code != 3)
    {
         fprintf(stderr, "ACK Packet %d had the wrong OP code: %d\n", i+1, ack->op_code);
    }
    if(ack->packet_num != -1) //tail is explicitly set to -1
    {
         fprintf(stderr, "ACK Packet # %d had the wrong packet number: %d\n", i+1, ack->packet_num);
    }

    printf("We sent this many packets: %d\n", i);
	printf("File transfer complete: %s\n", file_name);


    /*
    -------------------------------------------------
    Shutdown
    -------------------------------------------------
    */
	//free all that heap memory
	free(ack);
	free(packets);
	free(file_buffer);

    close(sfd); //close socket
    return 0;
}
