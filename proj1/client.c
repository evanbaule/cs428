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
#include <ctype.h>


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
    memset((char *)&client_addr, 0, sizeof(client_addr));
    
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    //Server will listen to 8080, clients will connect to ip:8080 to send to server
    client_addr.sin_port = htons(0);

    //Set IP 0.0.0.0/ANY
       //Zero out memory for safety
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
    struct sockaddr_in serv_addr;
    /* fill in the server's address and data */
    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(21235);
    //serv_addr.sin_port = port; 



    //printf("Attempting to establish connection to: %s:%d...\n", hostname, port);
    
    //Copy hostname addr into the server struct


     memcpy((void *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
    //memcpy((void*)&serv_addr.sin_addr, "")
    //serv_addr.sin_addr.s_addr = inet_addr("192.168.1.8");

    printf("ip post change:\t%s\n", inet_ntoa(serv_addr.sin_addr));
    printf("port:\t%d\n", serv_addr.sin_port);
    int connect_r = 0;
    if((connect_r = connect(client_sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) 
    {
	    printf("rv: %d\n", connect_r);
        perror("Failure connecting to aaaaaaaaaaaaaaaaaaaaaaaa server");
        exit(EXIT_FAILURE);
    }

    printf("connect_r: %d\n", connect_r);

    while(1)
    {
        //SENDING
        printf(">> ");
        char wbuf[256]; //message buffer
        fgets(wbuf, 256, stdin);

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

        char rbuf[256];
        read(client_sfd, rbuf, 256);
        printf("From server: %s", rbuf);
                
    }

    shutdown(client_sfd, SHUT_RDWR);
    return 0;
}
