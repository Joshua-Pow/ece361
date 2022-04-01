/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "../conferencing.h"

char users[5][10] = {"user1", "user2", "user3", "user4", "user5"};
char pass[5][10] = {"123", "123", "123", "123", "123"};
char sessions[5][20] = {"Null", "Null", "Null", "Null", "Null"};
int connected[5] = {0, 0, 0, 0, 0}; //1 = connected, 0 = not connected
int userfds[5] = {0, 0, 0, 0, 0};

void print_sessions(){
    for (int i = 0; i < 5; i++)
    {
        printf("Session %d: %s\n", i, sessions[i]);
    }
    
}

int valid_user(char* user){
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(user, users[i])==0){
            return i;
        }
    }
    return -1;   
}

int valid_sess(char* sess){
    for (int i = 0; i < 5; i++)
    {
        if (strcmp(sess, sessions[i])==0){
            return i;
        }
    }
    return -1; 
}

void login(struct message* packet, int fd){
    int valid = valid_user(packet->source); //If valid user, holds index for info

    if (valid!=-1){
        userfds[valid] = fd;
        if(connected[valid] == 1){ //user already logged in
            if (send(userfds[valid], "3:18:Server:Already logged in", strlen("3:18:Server:Already logged in")+1, 0) == -1) {
                perror("send");
            }
        }
        else if (strcmp(packet->data, pass[valid])!=0){
            if (send(userfds[valid], "3:30:Server:Invalid password for username", strlen("3:30:Server:Invalid password for username")+1, 0) == -1) {
                perror("send");
            }
        } 

        //Everything good, set to connected and return ack
        connected[valid] = 1;
        if (send(userfds[valid], "2:0:Server:", strlen("2:0:Server:")+1, 0) == -1) {
            perror("send");
        }

    }
    else if (valid==-1){
        if (send(fd, "3:19:Server:Invalid login name", strlen("3:19:Server:Invalid login name")+1, 0) == -1) {
            perror("send");
        }
    }
}

void leave_sess(struct message* packet){
    int valid_u = valid_user(packet->source);

    if (valid_u != -1 && strcmp(sessions[valid_u], "Null")!=0){
        strcpy(sessions[valid_u], "Null");
    }
}

void exitChat(struct message* packet){
    int valid = valid_user(packet->source); //If valid user, holds index for info

    if (valid!=-1){
        connected[valid] = 0; //Disconnect
        leave_sess(packet); //Leave session
    }
    //Do nothing if not valid user
}

void join(struct message* packet){
    print_sessions();
    int valid_s = valid_sess(packet->data); //If valid sess, holds index for info
    int valid_u = valid_user(packet->source); //If valid user, holds index for user
    char return_message[150];
    printf("join data: %s\n", packet->data);

    if (valid_s!=-1 && valid_u!=-1 && strcmp(sessions[valid_u], "Null")==0){
        printf("valid join session!\n");
        strcpy(sessions[valid_u], packet->data); //Sets the session for the user
        
        sprintf(return_message, "6:%d:Server:%s", strlen(packet->data), packet->data);
        printf("b4 joinack: %s\n", return_message);
        if (send(userfds[valid_u], return_message, strlen(return_message)+1, 0) == -1) {
            perror("send");
        }
    }
    else{
        sprintf(return_message, "7:%d, Invalid session:Server:%s", strlen(packet->data)+strlen(", Invalid session"), packet->data);
        if (send(userfds[valid_u], return_message, strlen(return_message)+1, 0) == -1) {
            perror("send");
        } 
    }
    print_sessions();
}

void new_sess(struct message* packet){
    int valid_u = valid_user(packet->source);

    printf("b4 new\n");
    if (valid_u != -1 && strcmp(sessions[valid_u], "Null")==0){
        printf("session: %s\n", packet->data);
        strcpy(sessions[valid_u], packet->data);
        if (send(userfds[valid_u], "10:0:Server:", strlen("10:0:Server:")+1, 0) == -1) {
            perror("send");
        } 
    }
    print_sessions();
}

void message(struct message* packet){
    int valid_u = valid_user(packet->source);
    char session[20];
    char return_message[120];
    printf("message data: %s\n", packet->data);

    if (valid_u!=-1){
        for (int i = 0; i < 5; i++)
        {
            if (i!=valid_u && strcmp(sessions[i], sessions[valid_u])==0){
                sprintf(return_message, "11:%d:%s:%s", packet->size, packet->source, packet->data);
                if (send(userfds[i], return_message, strlen(return_message)+1, 0) == -1) {
                    perror("send");
                } 
            }
        }
    
    }
    
}

void query(struct message* packet){
    int valid_u = valid_user(packet->source);
    char return_message[250];
    char allUsers[200] = "Online: ";
    
    for (int i=0; i<5; i++){
        if (connected[i]==1){
            strcat(allUsers, "\n");
            strcat(allUsers, users[i]); //Find all unique rooms
            strcat(allUsers, " ");
            strcat(allUsers, sessions[i]);
        }
    }

    if(valid_u!=-1){
        sprintf(return_message, "13:%d:Server:%s", strlen(allUsers), allUsers);
        printf("query return: %s\n", return_message);
        if (send(userfds[valid_u], return_message, strlen(return_message)+1, 0) == -1) {
            perror("send");
        } 
    }
}

void getUsername(char* data, char* dest, char* msg){
    printf("data: %s\n", data);

    //strok only takes in a char* so we need to copy into a temp char*
    char* string = (char*)malloc(1000*sizeof(char));
    strcpy(string, data);

    char* token = strtok(string,":");
	printf("dest: %s\n", token);
	strcpy(dest, token);

	token = strtok(NULL, ":");
	printf("msg: %s\n", token);
	strcpy(msg, token);

    free(string);
}

void dm(struct message* packet){
    //Data form: username:messageToUser
    //ie: BillBob:Hi Bill, hows it going!
    int valid_u = valid_user(packet->source);
    char dest[100];
    char msg[1000];
    char return_message[1500];
    int data_size = packet->size-strlen(dest)-1; //We need to take the dest username out of the data size

    if (valid_u!=-1){
        getUsername(packet->data, dest, msg);
        int valid_dest = valid_user(dest);
        if (valid_dest!=-1){
            sprintf(return_message, "14:%d:%s:%s", data_size, dest, msg);
            if (send(userfds[valid_dest], return_message, strlen(return_message)+1, 0) == -1) {
                perror("send");
            } 
        }
    }

 
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: server {port}\nMissing/too many arguments\n");
        exit(1);
    }

    const char* port = argv[1]; // port we're listening on
    printf("Port: %s\n", port);

    fd_set master;    // master file descriptor list
    fd_set read_fds;  // temp file descriptor list for select()
    int fdmax;        // maximum file descriptor number

    int listener;     // listening socket descriptor
    int newfd;        // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];    // buffer for client data
    int nbytes;

	char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;        // for setsockopt() SO_REUSEADDR, below
    int i, j, rv;

	struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);    // clear the master and temp sets
    FD_ZERO(&read_fds);

	// get us a socket and bind it
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(NULL, port, &hints, &ai)) != 0) {
		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	for(p = ai; p != NULL; p = p->ai_next) {
    	listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) { 
			continue;
		}
		
		// lose the pesky "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			continue;
		}

		break;
	}

	// if we got here, it means we didn't get bound
	if (p == NULL) {
		fprintf(stderr, "selectserver: failed to bind\n");
		exit(2);
	}

	freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for(;;) {
        read_fds = master; // copy it
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for(i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
					newfd = accept(listener,
						(struct sockaddr *)&remoteaddr,
						&addrlen);

					if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {    // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                            "socket %d\n",
							inet_ntop(remoteaddr.ss_family,
								get_in_addr((struct sockaddr*)&remoteaddr),
								remoteIP, INET6_ADDRSTRLEN),
							newfd);
                    }
                } else {
                    // handle data from a client
                    memset(buf, 0, 256);
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        for (int k=0; k<5; k++){
                            if (userfds[k]==i){
                                connected[k]=0;
                                strcpy(sessions[k], "Null");
                            }
                        }
                        printf("selectserver: socket %d hung up\n", i);
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
                        // we got some data from a client

                        //Parse the data in buffer
                        struct message packet;
                        packet_fill(&packet, buf, nbytes);
                        printf("Packet data: %s\n", packet.data);

                        if (packet.type == LOGIN){
                            login(&packet, i);
                        }
                        else if (packet.type == EXIT){
                            exitChat(&packet);
                        }
                        else if (packet.type == JOIN){
                            join(&packet);
                        }
                        else if (packet.type == LEAVE_SESS){
                            leave_sess(&packet);
                        }
                        else if (packet.type == NEW_SESS){
                            new_sess(&packet);
                        }
                        else if (packet.type == MESSAGE){
                            message(&packet);
                        }
                        else if (packet.type == QUERY){
                            query(&packet);
                        }
                        else if (packet.type == DM){
                            //Packet format: 14:size:SourceUser:Data
                            //Data format: DestUser:Message
                            //packet example: 14:size:Will:Bob:Hello Will, its me bob!
                            dm(&packet);
                        }
                        memset(buf, 0, 256);
                        // got error or connection closed by client
                        if (nbytes <= 0) {
                            // connection closed
                            for (int k=0; k<5; k++){
                                if (userfds[k]==i){
                                    connected[k]=0;
                                    strcpy(sessions[k], "Null");
                                }
                            }
                            printf("selectserver: socket %d hung up\n", i);
                        }

                        // for(j = 0; j <= fdmax; j++) {
                        //     // send to everyone!
                        //     if (FD_ISSET(j, &master)) {
                        //         // except the listener and ourselves
                        //         if (j != listener && j != i) {
                        //             if (send(j, buf, nbytes, 0) == -1) {
                        //                 perror("send");
                        //             }
                        //         }
                        //     }
                        // }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!
    
    return 0;
}