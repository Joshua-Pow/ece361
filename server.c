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

#define MAXBUFLEN 100

void* get_in_addr(struct sockaddr *sa);

//Source: "Beejs Guide To Network Programming pg.37"
int main(int argc, char const *argv[])
{
    const char* port = argv[1];
    printf("Port: %s\n", port);

    struct addrinfo hints; //Criteria to be returned from get_addr
    struct addrinfo *servinfo; //returned info from getaddrinfo about
    struct addrinfo *p;

    struct sockaddr_storage their_addr;

    char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP (SOCK_STREAM is TCP)
    hints.ai_flags = AI_PASSIVE; //accepts anything on the network?

    int rv; //return value
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    int sockfd; //socket file descriptor
    for(p = servinfo; p != NULL; p = p->ai_next) { //Might not need to loop through all packets
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("listener: bind");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "listener: failed to bind socket\n");
		return 2;
	}

	freeaddrinfo(servinfo);

	printf("listener: waiting to recvfrom...\n");

    int numbytes;
	addr_len = sizeof their_addr;
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
		perror("recvfrom");
		exit(1);
	}

	printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
	printf("listener: packet is %d bytes long\n", numbytes);
	buf[numbytes] = '\0';
	printf("listener: packet contains \"%s\"\n", buf);

	//Send back to their_addr
	if(strcmp(buf,"ftp") == 0){ //Checks to see if message is ftp
		if ((numbytes = sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
			perror("sendto");
			exit(1);
		}
	}
	else{
		if ((numbytes = sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
			perror("sendto");
			exit(1);
		}
	}

	close(sockfd);

	return 0;
}

//Get sockaddr, IPv4 or IPv6
void* get_in_addr(struct sockaddr *sa){
    //AF_INET is IPv4
    //AF_INET6 is IPv6
    if (sa->sa_family == AF_INET) {
        //sockaddr_in is the same size as sockaddr
        //source: https://www.gta.ufrj.br/ensino/eel878/sockets/sockaddr_inman.html
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
