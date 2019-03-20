#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>


//Print IP Address formatted
void
paddr(unsigned char *a)
{
        printf("%d.%d.%d.%d\n", a[0], a[1], a[2], a[3]);
}

int
main(int argc, char ** argv)
{
    char* hostname = "localhost"; //default
    int port = 0;
    if(argc == 2)
        hostname = argv[1]; //server hostname from execution params
    if(argc == 3)
        port = atoi(argv[2]);

    // printf("%d\n", argc);
    // printf("hostname:\t%s\n", hostname);
    printf("Setting up client...\n");

    //syscall create socket TCP/IPv4
    int client_sfd = socket(AF_INET, SOCK_STREAM, 0); assert(client_sfd != -1 && "Socket creation syscall failure.");
    //printf("socket_fd:\t%d\n", client_sfd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;

    //Server will listen to 8080, clients will connect to ip:8080 to send to server
    client_addr.sin_port = htons(0);

    //Set IP 0.0.0.0/ANY
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //Zero out memory for safety
    memset((char *)&client_addr, 0, sizeof(client_addr));
    int brv = bind(client_sfd, (struct sockaddr *)&client_addr, sizeof(client_addr)); //assert(brv != -1 && "Failure binding to port: 8080");
    if(brv == -1)
    { 
        printf("brv@fail:\t%d\n", brv);
        perror("Failure binding socket to port");
        exit(EXIT_FAILURE);
    }
    //printf("brv:\t%d\n", brv);

    struct hostent *hp;
    struct sockaddr_in serv_addr;
    /* fill in the server's address and data */
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    //serv_addr.sin_port = port; 

    hp = gethostbyname(hostname); //why is this a warning?
    //test hp
    int i;
    for (i=0; hp->h_addr_list[i] != 0; i++) 
    {
        paddr((unsigned char*) hp->h_addr_list[i]);
    }

    if (!hp) {
        fprintf(stderr, "could not obtain address of %s\n", hostname);
        return 0;
    }

    //printf("Attempting to establish connection to: %s:%d...\n", hostname, port);
    
    //Copy hostname addr into the server struct


   // memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    //memcpy((void*)&serv_addr.sin_addr, "")
    serv_addr.sin_addr.s_addr = inet_addr("10.0.2.15");

    printf("ip post change:\t%s\n", inet_ntoa(serv_addr.sin_addr));
    printf("port:\t%d\n", serv_addr.sin_port);
    int connect_r = 0;
    if (connect_r = connect(client_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
	    printf("rv: %d\n", connect_r);
        perror("Failure connecting to aaaaaaaaaaaaaaaaaaaaaaaa server");
        exit(EXIT_FAILURE);
    }
    
    //Connection established, begin communication

    shutdown(client_sfd, SHUT_RDWR);
    return 0;
}
