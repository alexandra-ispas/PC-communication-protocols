#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "helpers.h"

int receive_and_send(int connfd1, int connfd2)
{
    char buf[BUFLEN];
    int bytes_send;
    int bytes_remaining;
    int bytes_received = 0;


    bytes_received = recv(connfd1, buf, BUFLEN, 0);
    if (bytes_received != 0) {
        fprintf(stderr, "Received: %s", buf);
    }

    bytes_remaining = bytes_received;

    bytes_send = send(connfd2, buf, bytes_remaining, 0);

    return bytes_received;
}

void run_echo_server(int listenfd)
{
    struct sockaddr_in client_addr;

    int bytes_received;
    int connfd = -1;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    if(listen(listenfd, 1) < 0){
        printf("Eroare listen\n");
    }

    if (( connfd = accept(listenfd,  (struct sockaddr *) &(client_addr), &socket_len)) < 0){
         printf("Eroare accept\n");
    }

    do {
        bytes_received = receive_and_send(connfd, connfd);
    } while (bytes_received > 0);

    close(connfd);
    close(listenfd);
}

void run_chat_server(int listenfd)
{
    struct sockaddr_in client_addr1;
    struct sockaddr_in client_addr2;

    int bytes_received;
    int connfd1 = -1;
    int connfd2 = -1;
    socklen_t socket_len = sizeof(struct sockaddr_in);

    if(listen(listenfd, 2) < 0){
        printf("Eroare listen\n");
    }

    if ((connfd1 = accept(listenfd,  (struct sockaddr *) &(client_addr1), &socket_len)) < 0){
         printf("Eroare accept\n");
    }
    if ((connfd2 = accept(listenfd,  (struct sockaddr *) &(client_addr2), &socket_len)) < 0){
         printf("Eroare accept\n");
    }

    do {
        bytes_received = receive_and_send(connfd1, connfd2);

        if (bytes_received == 0)
            break;

        bytes_received = receive_and_send(connfd2, connfd1);
    } while (bytes_received > 0);

    close(connfd1);
    close(connfd2);
    close(listenfd);
}

int main(int argc, char* argv[])
{
    int listenfd = -1;
    struct sockaddr_in serv_addr;
    socklen_t socket_len = sizeof(struct sockaddr_in);


    if (argc != 3) {
        printf("\n Usage: %s <ip> <port>\n", argv[0]);
        return 1;
    }

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
		fprintf(stdout, "Fail to open socket!");
		exit(EXIT_FAILURE);
	}

    serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], &(serv_addr.sin_addr));
	socklen_t st = sizeof(serv_addr);


    if (bind(listenfd, (struct sockaddr *) &(serv_addr), st) == -1) {
		fprintf(stderr, "eroare");
		exit(EXIT_FAILURE);
	}

    run_echo_server(listenfd);
    // run_chat_server(listenfd);

    close(listenfd);

    return 0;
}
