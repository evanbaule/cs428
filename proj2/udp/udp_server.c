#include "packet.h"

int main(int argc, char **argv)
{
    int serv_port = 8080;
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

    printf("Setting up client...\n");

    //Open the UDP socket
    int sfd;
    if((sfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Failure creating socket\n");
        exit(EXIT_FAILURE);
    }

    //Server configuration
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; //IPv4
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Failure binding socket.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in remaddr;
    socklen_t re_len = sizeof(remaddr);

    int recvlen = 0;
    char buff[2048];
    for (;;) {
        printf("waiting on port %d\n", serv_port);
        recvlen = recvfrom(sfd, buff, 2048, 0, (struct sockaddr *)&remaddr, &re_len);
        printf("received %d bytes\n", recvlen);
        if (recvlen > 0) {
                buff[recvlen] = 0;
                printf("received message: \"%s\"\n", buff);
        }
    }

    return 0;
}
