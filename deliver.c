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
#include <libgen.h>
#include <math.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

#include "packet.h"

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
    
    //Measure RTT time
    clock_t start = clock();

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

    struct packet pack;

    //https://linux.die.net/man/3/basename
    pack.filename = basename(filepath);

    struct stat fileinfo;
    stat(filepath, &fileinfo);
    pack.total_frag = ceil(fileinfo.st_size/1000);

    if (pack.total_frag < fileinfo.st_size/1000.0) {
        pack.total_frag+=1;
    }

    pack.frag_no = 0;

    int remaining = fileinfo.st_size;
    char filechar[1000];
    char* serialized;

    while (remaining != 0) {
        pack.frag_no+=1;

        if (remaining >= 1000) {
            remaining-=1000;
            pack.size = 1000;
        } else {
            pack.size = remaining;
            remaining = 0;
        }

        char tempTotalFrag[1000000];
        char tempFragNo[1000000];
        char tempSize[100];

        sprintf(tempTotalFrag, "%d", pack.total_frag);
        sprintf(tempFragNo, "%d", pack.frag_no);
        sprintf(tempSize, "%d", pack.size);

        serialized = (char*)malloc((strlen(tempTotalFrag) + strlen(tempFragNo) + strlen(tempSize) + strlen(pack.filename) + pack.size + 4)*sizeof(char));
        sprintf(serialized, "%d:%d:%d:%s:", pack.total_frag, pack.frag_no, pack.size, pack.filename);
        fread(pack.filedata, pack.size, 1, file);
        memcpy(serialized + (strlen(tempTotalFrag) + strlen(tempFragNo) + strlen(tempSize) + strlen(pack.filename) + 4), pack.filedata, pack.size);
        //printf("%s\n", serialized);

        if ((numbytes = sendto(sockfd, serialized, (strlen(tempTotalFrag) + strlen(tempFragNo) + strlen(tempSize) + strlen(pack.filename) + pack.size + 4), 0, (struct sockaddr*)&dest, sizeof(dest))) == -1) {
            perror("client: sendto");
            exit(1);
        }

        if ((numbytes = recvfrom(sockfd, message, strlen(message), 0, p->ai_addr, &p->ai_addrlen)) == -1) {
            perror("client: recvfrom");
            return 0;
        }

        if (pack.frag_no==1){
            clock_t end = clock();
            printf("RTT time: %f microseconds\n", ((double)(end-start)/CLOCKS_PER_SEC)*1000000);
        }

        if (strcmp(message, "Ack") != 0) {
            printf("File transfer could not be completed.\n");
            printf("%s\n", message);
            free(serialized);
            break;
        }

        free(serialized);
        serialized = NULL;
    }

    fclose(file);
    freeaddrinfo(servinfo);

    return 0;
}
