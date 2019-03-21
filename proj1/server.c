#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

void
paddr(unsigned char *a)
{
        printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

int
main(int argc, char const ** argv)
{
    int serv_port = 21235;
    if(argc == 2)
    {
        serv_port = atoi(argv[1]);
    }

    printf("Starting server - listening on port: %d...\n", serv_port);

    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
	socklen_t alen;       /* length of address structure */
    int rqst; //for accept
    int sockoptval = 1;

    //syscall create socket TCP/IPv4
    int serv_sfd;
    if((serv_sfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Failure creating socket\n");
        exit(EXIT_FAILURE);
    }
    printf("Server: socket_fd:\t%d\n", serv_sfd);

    // allow immediate reuse of the port
	setsockopt(serv_sfd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int));

    //syscall modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
    //Set IP 0.0.0.0/ANY
    printf("Server port: %d\n", serv_addr.sin_port);
    printf("Server IP address is: %s\n", inet_ntoa( serv_addr.sin_addr));

    
    if(bind(serv_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {   
        printf("Failure to bind\n");
        exit(EXIT_FAILURE);
    } 

    printf("Listening for connections...\n");
    if(listen(serv_sfd, 3) < 0)
    {
        perror("Failure listening to socket.");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while(1)
    {   
        printf("hello?????\n");
        while((rqst = accept(serv_sfd, (struct sockaddr*) &client_addr, &alen)) < 0)
        {
            printf("Connection failed...");
            break;
        }
        printf("Listening to socket: %d for cycle %d\n", serv_sfd, i);
        i++;
    }



    //close socket
    shutdown(serv_sfd, SHUT_RDWR);

    return 0;
}

