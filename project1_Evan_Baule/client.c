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
#include <arpa/inet.h> //only used for printing out IP addresses during development
#include <ctype.h>

int
main(int argc, char ** argv)
{
    char* hostname = "localhost"; //default
    int port = 0;
    if(argc == 2)
        hostname = argv[1]; //server hostname from execution params
    if(argc == 3)
        port = atoi(argv[2]);

    printf("Setting up client...\n");

    //syscall create socket TCP/IPv4
    int client_sfd = socket(AF_INET, SOCK_STREAM, 0); assert(client_sfd != -1 && "Socket creation syscall failure.");
    //printf("socket_fd:\t%d\n", client_sfd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");
   
    struct sockaddr_in client_addr;
    memset((char *)&client_addr, 0, sizeof(client_addr)); //zero out memory for safety
    client_addr.sin_family = AF_INET; //IPv4
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); //client address doesn't really matter
    client_addr.sin_port = htons(0); //client port doesn't really matter either

    int brv = bind(client_sfd, (struct sockaddr *)&client_addr, sizeof(client_addr)); //assert(brv != -1 && "Failure binding to port: 8080");
    if(brv == -1)
    { 
        printf("brv@fail:\t%d\n", brv);
        perror("Failure binding socket to port");
        exit(EXIT_FAILURE);
    }
    //printf("brv:\t%d\n", brv);

    struct hostent *hp;
    hp = gethostbyname(hostname); //why is this a warning?
    if (!hp) {
        fprintf(stderr, "could not obtain address of %s\n", hostname);
        return 0;
    }

    //Configure server details
    struct sockaddr_in serv_addr;
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    
    //Copy hostname ip addr into the server struct
    memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    // printf("ip post change:\t%s\n", inet_ntoa(serv_addr.sin_addr));
    // printf("port:\t%d\n", serv_addr.sin_port);

    printf("Attempting to establish connection to: %s:%d...\n", hostname, port);
    int connect_r = 0;
    if((connect_r = connect(client_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) 
    {
        perror("Failure connecting to server");
        exit(EXIT_FAILURE);
    }

    while(1)
    {
        //SENDING
        printf(">> ");
        char wbuf[256]; //message buffer
        fgets(wbuf, 256, stdin);

        //wbuf2 is only used for checking for exit messages
        char wbuf2[256]; //Make a copy of our message
        memcpy(&wbuf2, wbuf, 256);
        for(int i = 0; wbuf2[i]; i++){
            wbuf2[i] = tolower(wbuf2[i]); //convert message to lowercase for comparison
        }
        //if we enter exit, EXIT, Exit, etc. then we break the connection
        if(strcmp(wbuf2, "exit\n") == 0) //include the '\n' char because fgets includes it in the buffer
        {
            write(client_sfd, wbuf2, 256);
            printf("Closing connection...\n");
            break;
        }
        else
        {
            //printf("Sending msg: %s\n", wbuf);
            write(client_sfd, wbuf, 256);
        }

        //RECIEVING
        char rbuf[256];
        read(client_sfd, rbuf, 256); //blocking read, waits for a response
        if(strcmp(rbuf, "exit\n") == 0)
        {
            printf("Server disconnected...\n");
            break;
        }
        else
            printf("From {Server}: %s", rbuf);
    }
    
    //Closing socket before shutting down
    shutdown(client_sfd, SHUT_RDWR);
    return 0;
}
