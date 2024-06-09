#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdbool.h>
#include <string.h>
#include <netdb.h>

int main(int argc, char* argv[]) {
    bool debug = false;
    int PORT = 20241;
    char* HOST = "os4.iot.dslab.ds.open-cloud.xyz";

    if (argc <= 6) {
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], "--debug") == 0) debug = true;
            else if (strcmp(argv[i], "--port") == 0) PORT = atoi(argv[i + 1]);
            else if (strcmp(argv[i], "--host") == 0) HOST = argv[i + 1];
        }
    } else {
        fprintf(stderr, "Usage: %s [--debug] [--port PORT] [--host HOST]\n", argv[0]);
        exit(1);
    }

    int domain = AF_INET;
    int type = SOCK_STREAM;

    int sd = socket(domain, type, 0);
    if (sd < 0) {
        perror("socket");
        exit(1);
    }

    struct hostent *host_ip = gethostbyname(HOST);
    if (!host_ip) {
        fprintf(stderr, "Error resolving host %s\n", HOST);
        exit(1);
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    memcpy(&sin.sin_addr.s_addr, host_ip->h_addr_list[0], host_ip->h_length);

    if (connect(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1) {
        perror("connect");
        exit(1);
    }

    int retval;
    fd_set rfds;
    int nfds = sd;

    char buff[1024];
    ssize_t s;

    while (1) {
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);    // terminal
        FD_SET(sd, &rfds);   // socket

        retval = select(nfds + 1, &rfds, NULL, NULL, NULL);
        if (retval == -1) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(0, &rfds)) {
            s = read(0, buff, sizeof(buff));
            if (s == -1) {
                perror("read");
                exit(1);
            }

            // anagnwrizei to pattern "N name surname reason"
            int N; //int gia ton ari8mo mprosta apo to onoma
            char name[256], surname[256], reason[256]; 
			//sscanf se afhnei na diavaseis data apo string, kai diavazei character by character.
            if (sscanf(buff, "%d %255s %255s %255s", &N, name, surname, reason) == 4) { //==4 gia na einai akrivws ta arguments, alliws error
                // ta name surname reason stelnontai ston server
                snprintf(buff, sizeof(buff), "%d %s %s %s", N, name, surname, reason);
				//The snprintf() function formats and stores a series of characters and values in the array buffer. (apo geeksforgeeks site)
                if (debug) printf("[DEBUG] sent '%s'\n", buff);
                if (write(sd, buff, strlen(buff)) == -1) {
                    perror("write");
                    exit(1);
                }

                // read to response tou server gia to verif. code
                char serverdata[1024];
                ssize_t s2 = read(sd, serverdata, sizeof(serverdata));
                if (s2 == -1) {
                    perror("read");
                    exit(1);
                }
                if (debug) printf("[DEBUG] read '%s'\n", serverdata);

                
                printf("Send verification code: '%s'\n", serverdata);

                // Read the verification code from the user
                s = read(0, buff, sizeof(buff));
                if (s == -1) {
                    perror("read");
                    exit(1);
                }

                // Send the verification code to the server
                if (debug) printf("[DEBUG] sent '%s'\n", buff);
                if (write(sd, buff, s) == -1) {
                    perror("write");
                    exit(1);
                }

                // Read the server's response after verification
                s2 = read(sd, serverdata, sizeof(serverdata));
                if (s2 == -1) {
                    perror("read");
                    exit(1);
                }
                if (debug) printf("[DEBUG] read '%s'\n", serverdata);

                // Check if the server response is ACK or an error message
                if (strncmp(serverdata, "ACK", 3) == 0) {
                    printf("%s\n", serverdata);
                } else {
                    printf("Error: %s\n", serverdata);
                }

            } else if (strcmp(buff, "help") == 0) {
                printf("help message...\n");
            } else if (strcmp(buff, "exit") == 0) {
                close(sd);
                exit(0);
            } else {
                if (debug) printf("[DEBUG] sent '%s'\n", buff);
                if (write(sd, buff, s) == -1) {
                    perror("write");
                    exit(1);
                }
            }
        } else if (FD_ISSET(sd, &rfds)) {
            // Handle input from the socket
            char serverdata[1024];
            ssize_t s2 = read(sd, serverdata, sizeof(serverdata));
            if (s2 == -1) {
                perror("read");
                exit(1);
            }
            if (debug) printf("[DEBUG] read '%s'\n", serverdata);
        }
    }

    return 0;
}

