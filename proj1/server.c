#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

int
main(int argc, char const ** argv)
{
    //syscall create socket TCP/IPv4
    int serv_sfd = socket(PF_INET, SOCK_STREAM, 0); assert(serv_sfd != -1 && "Socket creation syscall failure.");
    printf("socket_fd:\t%d\n", serv_sfd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = PF_INET;

    //Server will listen to 8080, clients will connect to localhost:8080 to send to server
    int serv_port = 8080;
    serv_addr.sin_port = htons(serv_port);

    //Set IP 0.0.0.0/ANY
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Zero out memory for safety
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    int brv = bind(serv_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) < 0); //assert(brv != -1 && "Failure binding to port: 8080");
    if(brv == -1)
    {
        printf("brv@fail:\t%d\n", brv);
        perror("Failure binding socket to port: 8080");
        exit(EXIT_FAILURE);
    }

    return 0;
}

