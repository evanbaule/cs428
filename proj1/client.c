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

int
main(int argc, char const ** argv)
{
    char* hostname = ""; //default
    int port;
    if(argc == 2)
        hostname = argv[1]; //server hostname from execution params
    else hostname = "localhost";

    if(argc == 3)
        port = atoi(argv[2]);
    else port = 8080;


    
    // printf("%d\n", argc);
    // printf("hostname:\t%s\n", hostname);

    //syscall create socket TCP/IPv4
    int client_sfd = socket(PF_INET, SOCK_STREAM, 0); assert(client_sfd != -1 && "Socket creation syscall failure.");
    //printf("socket_fd:\t%d\n", client_sfd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    struct sockaddr_in client_addr;
    client_addr.sin_family = PF_INET;

    //Server will listen to 8080, clients will connect to ip:8080 to send to server
    int client_port = 8080;
    client_addr.sin_port = htons(client_port);

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
    serv_addr.sin_family = PF_INET;
    serv_addr.sin_port = htons(port);

    hp = gethostbyname(hostname); //why is this a warning?
    if (!hp) {
        fprintf(stderr, "could not obtain address of %s\n", hostname);
        return 0;
    }

    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    if (connect(client_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
	    perror("Failure connecting to server");
        exit(EXIT_FAILURE);
    }
    //CONNECTION ESTABLISHED
    /*  
    once a connection is established:
        1. the sever will send an ID back to the client that will be used to identify it
        2. the client will store that ID and preface each message with the ID 
            so that the server knows where each message came from
        3. idk how the server differentiates between clients when responding (apparently it doesnt have to)

    */


    return 0;
}

