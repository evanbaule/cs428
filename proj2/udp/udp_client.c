#include "packet.h"

int main(int argc, char **argv)
{
    /* code */
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

    //Open the UDP socket
    int sfd;
    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failure creating socket\n");
        exit(EXIT_FAILURE);
    }

    //Set up destination server details
    struct hostent *hp;
    hp = gethostbyname(hostname); //returns IP & server details
    if(!hp)
    {
        perror("Failure getting IP from given hostname.\n");
        exit(EXIT_FAILURE);
    }

    //Server configuration
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(port);
    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);

    if(bind(sfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Failure binding socket.\n");
        exit(EXIT_FAILURE);
    }

    char* msg = "hello";
    //write msg to socket (packets)
    if(sendto(sfd, msg, strlen(msg), 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Failed sending msg to host");
        exit(EXIT_FAILURE);
    }

    return 0;
}