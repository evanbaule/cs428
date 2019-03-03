#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>


int
main(int argc, char const ** argv)
{
    //syscall create socket TCP/IPv4
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    printf("sfd:\t%d\n", socket_fd);

    return 0;
}

