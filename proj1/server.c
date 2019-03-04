#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>

int
main(int argc, char const ** argv)
{
    //syscall create socket TCP/IPv4
    int ss_fd = socket(PF_INET, SOCK_STREAM, 0); assert(ss_fd != -1 && "Socket creation syscall failure.");
    printf("socket_fd:\t%d\n", ss_fd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    struct sockaddr_in* saddr;
    saddr->sin_family = PF_INET;
    saddr->sin_port = htons(8080);
    //saddr->sin_addr = htonl(INADDR_ANY);
    //syscall bind socket to 
    int brv = bind(ss_fd, (struct sockaddr *)saddr, sizeof(saddr) < 0);
    if(brv == -1)
    {
      printf("we fucked it:\t%d\n", brv);
      perror("we fucked it heres the errno");
    }

    return 0;
}

