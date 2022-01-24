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

//Source: "Beejs Guide To Network Programming pg.37"
//Format: deliver {server ip} {port}
int main(int argc, char const *argv[])
{
    int sockfd;
    struct addrinfo hints; //Criteria to be returned from get_addr
    struct addrinfo* servinfo; //returned info from getaddrinfo about
    struct addrinfo* p;
    int rv;
    int numbytes;

    if (argc != 3) {
        fprintf(stderr, "usage: Missing/too many arguments\n");
        exit(1);
    }

    char temp[10];
    char filepath[1000];
    //Format: ftp {filepath}
    scanf("%s%s", temp, filepath);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    if ((rv = getaddrinfo(NULL, "6969", &hints, &servinfo)) != 0) { //Fills up servinfo
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) { //Might not need to loop through all
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { //returns file descriptor for open socket
            perror("client: socket");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }

    //Try to open file given by user
    FILE* file = fopen(filepath, "r");

    if (strcmp(temp, "ftp") != 0) {
        printf("Invalid command\n");
        return 0;
    }

    //If file couldn't be opened, exit
    if (file == NULL) {
        printf("Invalid file\n");
        return 0;
    } 

    //Message to send since file was found
    char message[] = "ftp";

    //Set up where to send the message
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));

    inet_pton(AF_INET, argv[1], &(dest.sin_addr));
    
    //Send message, but if -1 is returned, exit
    if ((numbytes = sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&dest, sizeof(dest))) == -1) {
        perror("client: sendto");
        exit(1);
    }

    //Receive message from server, but if -1 is returned, exit
    if ((numbytes = recvfrom(sockfd, message, strlen(message), 0, p->ai_addr, &p->ai_addrlen)) == -1) {
        perror("client: recvfrom");
        return 0;
    }

    //If message is == "yes" tell the user that files can be transferred
    if (strcmp(message, "yes") == 0) {
        printf("A file transfer can start.\n");
    }

    freeaddrinfo(servinfo);

    return 0;
}
