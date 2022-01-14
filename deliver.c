#include <stdio.h>
#include <arpa/inet.h>

//deliver 128.100.13.65 3550
int main(int argc, char const *argv[])
{
    printf("IP: %s\n", argv[1]);
    printf("Port: %s\n", argv[2]);

    struct sockaddr_in socketAddr;
    char str[INET_ADDRSTRLEN];

    inet_pton(AF_INET, argv[1], &(socketAddr.sin_addr));

    return 0;
}
