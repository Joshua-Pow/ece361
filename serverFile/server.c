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

#include "packet.h"

#define MAXBUFLEN 5000

void* get_in_addr(struct sockaddr *sa);
double uniform_rand();

//Source: "Beejs Guide To Network Programming pg.37"
//Format: server {port}
int main(int argc, char const *argv[])
{
	// struct packet temp;
	// packet_fill(&temp,"3:1:10:foobar.txt:lo World!\n");

	if (argc != 2) {
        fprintf(stderr, "usage: server {port}\nMissing/too many arguments\n");
        exit(1);
    }

    const char* port = argv[1];
    printf("Port: %s\n", port);

    struct addrinfo hints; //Criteria to be returned from get_addr
    struct addrinfo *servinfo; //returned info from getaddrinfo about
    struct addrinfo *p;

    struct sockaddr_in their_addr;

    char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_DGRAM; //UDP (SOCK_STREAM is TCP)
    hints.ai_flags = AI_PASSIVE; //accepts anything

    int rv; //return value
    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) { //Fills up servinfo
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    int sockfd; //socket file descriptor
    for(p = servinfo; p != NULL; p = p->ai_next) { //Might not need to loop through all
		if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { //returns file descriptor for open socket
			perror("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { //binds the ip to the port number
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

	//Loop the server to keep receiving
	FILE* file; //file to write to
	int recvFtp = 0; //holds value to determine if FTP was already sent

	while (1){
		memset(buf, 0, sizeof(buf)); //clear the buffer
		printf("listener: waiting to recvfrom...\n");

		//Recieve packet from client
		int numbytes;
		addr_len = sizeof their_addr;
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}

		printf("listener: got packet from %s\n", inet_ntop(their_addr.sin_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
		printf("listener: packet is %d bytes long\n", numbytes);

		//Draw a random number to determine whether or not to process the packet
		if (uniform_rand() > 0.01){

			//Checks to see if message is ftp
			if(recvFtp == 1) { //FTP was already sent
				printf("listener: packet contains \"%x\"\n", buf);

				//info from the packet recieved from client
				struct packet info;
				packet_fill(&info, buf, numbytes);

				//If its the first packet open file and save file pointer
				if (info.frag_no == 1){ 
					file = fopen(info.filename, "wr"); //make sure to close later
					// if (file==NULL){ //If file couldnt be opened 
					// 	if ((numbytes = sendto(sockfd, "Error", strlen("Error"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
					// 		perror("Cant open file");
					// 		exit(1);
					// 	}
					// }
				}

				//Puts data into file
				fwrite(info.filedata, sizeof(char), info.size, file);
				printf("Writing 2 file\n");

				//If last packet close file
				if(info.frag_no==info.total_frag){
					fclose(file);
					recvFtp = 0; //reset the recieved ftp flag
				}


				//Send an acknowledge that we got the packet
				if ((numbytes = sendto(sockfd, "Ack", strlen("Ack"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
					perror("sendto - Ack");
					exit(1);
				}
			}
			else if(strcmp(buf,"ftp") == 0){ //Is FTP in the buffer?
				buf[numbytes] = '\0';
				printf("listener: packet contains \"%s\"\n", buf);
				recvFtp = 1;
				if ((numbytes = sendto(sockfd, "yes", strlen("yes"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
					perror("sendto - Yes");
					exit(1);
				}
			}
			else{
				if ((numbytes = sendto(sockfd, "no", strlen("no"), 0, (struct sockaddr *)&their_addr, addr_len)) == -1) {
					perror("sendto - No");
					exit(1);
				}
			}
		}
		else{
			printf("Server: Dropping packet\n");
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

double uniform_rand(){
	double random = rand()/(double) RAND_MAX;
	printf("random number: %f\n", random);
	return random;
}
