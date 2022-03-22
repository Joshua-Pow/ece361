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
#include <sys/time.h>
#include <stdbool.h>

#include "../conferencing.h"


void leavesession(int sockfd, char* username) {
    int nbytes;
    char pack[2000];
    sprintf(pack, "8:0:%s:", username);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }

    printf("You have left the session.\n");
}

void logout(int sockfd, char* username, bool in_session) {
    if (in_session) {
        leavesession(sockfd, username);
    }

    int nbytes;
    char pack[2000];
    sprintf(pack, "4:0:%s:", username);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }

    close(sockfd);
    printf("You are now logged out.\n");
}

void joinsession(int sockfd, char* username, char* session) {
    int nbytes;
    char pack[2000];
    sprintf(pack, "5:%d:%s:%s", strlen(session), username, session);
    printf("pack: %s", pack);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }
}

void createsession(int sockfd, char* username, char* session) {
    int nbytes;
    char pack[2000];
    sprintf(pack, "9:%d:%s:%s", strlen(session), username, session);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }
}

void list(int sockfd, char* username) {
    int nbytes;
    char pack[2000];
    sprintf(pack, "12:0:%s:", username);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }
}

void text(int sockfd, char* username, char* message) {
    int nbytes;
    char pack[2000];
    sprintf(pack, "11:%d:%s:%s", strlen(message), username, message);

    if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
        perror("send");
    }
}

int login(char* username) {
    char temp = ' ';
    bool loggedin = false;
    int sockfd;

    while(loggedin == false) {
        char password[20];
        char server_ip[] = "128.100.13.156";
        char server_port[5];

        char initial_input[1000];

        fgets(initial_input, 1000, stdin);
        initial_input[strlen(initial_input) - 1] = '\0';
        char* split_input = strtok(initial_input, " ");

        if (strcmp(split_input, "/quit") == 0 || strcmp(split_input, "/quit\n") == 0) {
            printf("Quitting text conferencing.\n");
            return -1;
        } else if (strcmp(split_input, "/login") != 0) {
            if (strcmp(split_input, "/login\n") == 0) {
                printf("Missing arguments. Try again.\n");
                continue;
            } else {
                printf("Unable to use other commands until you are logged in.\n");
                continue;
            }
        } else {
            int count = 0;
            bool error = false;

            while (true) {
                split_input = strtok(NULL, " ");

                if (split_input != NULL) {
                    //printf("tok: %s\n", split_input);
                    if (count == 0) {
                        strcpy(username, split_input);
                        count++;
                    } else if (count == 1) {
                        strcpy(password, split_input);
                        count++;
                    } else if (count == 2) {
                        //strcpy(server_ip, split_input);
                        //printf("serverip in tok: %s\n", server_ip);
                        count++;
                    } else if (count == 3) {
                        //printf("IP2: %s\n", server_ip);
                        strcpy(server_port, split_input);
                        count++;
                    } else {
                        printf("Too many arguments. Try again.\n");
                        error = true;
                        break;
                    }
                } else if (count == 4) {
                    break;
                } else {
                    printf("Too few arguments. Try again.\n");
                    error = true;
                    break;
                }
            }

            if (error == true) {
                continue;
            }
            
            split_input = strtok(NULL, " ");
            strcpy(username, split_input);
            split_input = strtok(NULL, " ");
            strcpy(password, split_input);
            split_input = strtok(NULL, " ");
            strcpy(server_ip, split_input);
            split_input = strtok(NULL, " ");
            strcpy(server_port, split_input);

            if (username == NULL || password == NULL || server_ip == NULL || server_port == NULL) {
                printf("Too few arguments. Try again.\n");
                continue;
            }

            split_input = strtok(NULL, " ");

            if (split_input != NULL) {
                printf("Too few arguments. Try again.\n");
                continue;
            } 

            char server_ip2[] = "128.100.13.156";
            struct addrinfo hints;
            struct addrinfo* serv_info;
            struct addrinfo* p;
            int rv;
            
            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            //hints.ai_flags = AI_PASSIVE;
            printf("IP2: %s\nPort: %s\nUser: %s\nPass: %s\n", server_ip2, server_port, username, password);
            if ((rv = getaddrinfo("128.100.13.156", "6666", &hints, &serv_info)) != 0) {
                printf("server_port: %s\n", server_port);
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                exit(2);
            }

            for (p = serv_info; p != NULL; p = p->ai_next) { //Might not need to loop through all
                if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { //returns file descriptor for open socket
                    perror("client: socket");
                    continue;
                }

                if ((rv = connect(sockfd, p->ai_addr, p->ai_addrlen)) == -1) {
                    //fprintf(stderr, "connect: failed to connect to server\n");
                    perror("connect");
                    continue;
                }

                break;
            }

            if (p == NULL) {
                fprintf(stderr, "client: failed to create socket\n");
                exit(3);
            }

            freeaddrinfo(serv_info);

            int nbytes;
            char pack[2000];
            sprintf(pack, "1:%d:%s:%s", strlen(password), username, password);

            if ((nbytes = send(sockfd, pack, strlen(pack), 0)) == -1) {
                fprintf(stderr, "send: failed to send to server\n");
                exit(5);
            }

            char recv_pack[2000];

            if ((nbytes = recv(sockfd, recv_pack, sizeof(recv_pack), 0)) == -1) {
                fprintf(stderr, "send: failed to receive from server\n");
                exit(6);
            }

            struct message packet;
            packet_fill(&packet, recv_pack, nbytes);

            if (packet.type == LO_ACK) {
                printf("You have successfully logged in.\n");
                loggedin = true;
            } else {
                printf("You have failed to login due to %s. Try again.", packet.data);
                continue;
            }
        }
    }

    return sockfd;
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stderr, "usage: Missing/too many arguments\n");
        exit(1);
    }

    printf("Login to continue\n");

    bool loggedin = false;
    char username[20];
    int sockfd;
    char temp = ' ';

    sockfd = login(username);

    if (sockfd == -1) {
        return 0;
    }

    loggedin = true;

    bool in_session = false;
    int fdmax;

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    FD_SET(sockfd, &read_fds);

    if (sockfd > STDIN_FILENO) {
        fdmax = sockfd;
    } else {
        fdmax = STDIN_FILENO;
    }

    while (true) {
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(7);
        }

        if (loggedin == false) {
            sockfd = login(username);

            if (sockfd == -1) {
                return 0;
            }

            loggedin = true;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (loggedin) {
                char initial_input[1000];

                fgets(initial_input, 1000, stdin);
                initial_input[strlen(initial_input) - 1] = '\0';
                char* split_input = strtok(initial_input, " ");

                if (split_input == NULL) {
                    split_input = initial_input;
                }

                if (strcmp(split_input, "/login\n") == 0 || strcmp(split_input, "/login") == 0) {
                    printf("You are already logged in.\n");
                } else if (strcmp(split_input, "/logout\n") == 0 || strcmp(split_input, "/logout") == 0) {
                    logout(sockfd, username, in_session);
                    loggedin = false;
                    in_session = false;
                    close(sockfd);
                } else if (strcmp(split_input, "/joinsession\n") == 0 || strcmp(split_input, "/joinsession") == 0) {
                    if (in_session) {
                        printf("You are already in a session.\n");
                    } else {
                        split_input = strtok(NULL, " ");

                        if (split_input != NULL) {
                            printf("split_input: %s\n",split_input);
                            joinsession(sockfd, username, split_input);
                        } else {
                            printf("Missing session ID. Try again.\n");
                        }
                    }
                } else if (strcmp(split_input, "/leavesession\n") == 0 || strcmp(split_input, "/leavesession") == 0) {
                    if (in_session) {
                        leavesession(sockfd, username);
                        in_session = false;
                    } else {
                        printf("You are not currently in a session.\n");
                    }
                } else if (strcmp(split_input, "/createsession\n") == 0 || strcmp(split_input, "/createsession") == 0) {
                    if (in_session) {
                        printf("You are already in a session.\n");
                    } else {
                        split_input = strtok(NULL, " ");

                        if (split_input != NULL) {
                            printf("split_input: %s\n",split_input);
                            createsession(sockfd, username, split_input);
                        } else {
                            printf("Missing session ID. Try again.\n");
                        }
                    }
                } else if (strcmp(split_input, "/list\n") == 0 || strcmp(split_input, "/list") == 0) {
                    list(sockfd, username);
                } else if (strcmp(split_input, "/quit\n") == 0 || strcmp(split_input, "/quit") == 0) {
                    logout(sockfd, username, in_session);
                    in_session = false;
                    loggedin = false;
                    close(sockfd);
                    return 0;
                } else {
                    if (in_session) {
                        text(sockfd, username, initial_input);
                    } else {
                        printf("Invalid command. Try again.\n");
                    }
                }
            } else {
                
            }
        } else if (FD_ISSET(sockfd, &read_fds)) {
            char recv_pack[2000];
            int nbytes;

            if ((nbytes = recv(sockfd, recv_pack, sizeof(recv_pack), 0)) == -1) {
                fprintf(stderr, "send: failed to receive from server\n");
                exit(8);
            }

            struct message packet;
            printf("packet: %s\n", recv_pack);
            packet_fill(&packet, recv_pack, nbytes);

            if (packet.type == JN_ACK) {
                printf("Successfully joined session %s\n", packet.data);
                in_session = true;
            } else if (packet.type == JN_NAK) {
                printf("Unable to join session due to %s. Try again.\n", packet.data);
            } else if (packet.type == NS_ACK) {
                printf("Successfully created and joined session.\n");
                in_session = true;
            } else if (packet.type == MESSAGE) {
                printf("%s: %s\n", packet.source, packet.data);
            } else if (packet.type == QU_ACK) {
                printf("%s\n", packet.data);
            }
        }
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);
    }

    close(sockfd);
}