#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

//deliver 128.100.13.65 3550
int main(int argc, char const *argv[])
{
    int sockfd;
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    int rv;
    int numbytes;

    char* message[4]; //temp

    if (argc != 3) {
        fprintf(stderr, "usage: Missing/too many arguments\n");
        exit(1);
    }

    char filepath[1000];
    scanf("%s%s", filepath);

    const char* serverAddress = argv[1];
    const char* serverPort = argv[2];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; //IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if ((rv = getaddrinfo(serverAddress, serverPort, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }

    FILE* file = fopen(filepath, "r");

    if (file) {
        message = "ftp"
    } else {
        return 0;
    }

    if ((numbytes = sendto(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(1);
    }

    if ((numbytes = recvfrom(sockfd, message, strlen(message), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        return 0;
    }

    if (strcmp(message, "yes") == 0) {
        fprintf("A file transfer can start.\n")
    }

    freeaddrinfo(servinfo);

    return 0;
}
