#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>

int
main(int argc, char const ** argv)
{
    //syscall create socket TCP/IPv4
    int ss_fd = socket(PF_INET, SOCK_STREAM, 0); assert(ss_fd != -1 && "Socket creation syscall failure.");
    printf("socket_fd:\t%d\n", ss_fd);

    //syscall setsockopt to modify socket construction
    //int sso_rv = setsockopt(ss_fd, ); assert(sso_rv != -1 && "Setsockopt syscall failure to modify socket properties.");

    struct socket_addr s_addr;
    //syscall bind socket to 

    return 0;
}

