#include "helper.h"

void usage(char *file){
	fprintf(stderr, "Usage: %s server_address server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	/* disable buffering */
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	/* check if program has enough input arguments */
	if (argc < 3) {
		usage(argv[0]);
	}

	int sockfd, ret, flag = 1;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN], tcp_buff[TCP_SIZE];
	TCP_Packet *tcp_packet;
	fd_set read_fds, tmp_fds;	
	Action action;

	/* empty 'read_fds' set and 'temp_fds' set */
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	/* open socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "Failed to connect to socket.\n");

	/* add socket into the read_fds set */
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds);

	/* initialize data for socket */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton failed.\n");

	/* ask to connect to server */
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "Failed to connect.\n");

	/* send subscriber's ID-ul */
	ret = send(sockfd, argv[1], strlen(argv[1]) + 1, 0);
	DIE(ret < 0, "Failed to send subscriber's ID.\n");

	/* disabe Nagle's algorithm for this socket */
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

	while (1) {
		tmp_fds = read_fds;
		ret = select(sockfd + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "Failed to select a socket.\n");

		if (FD_ISSET(0, &tmp_fds)) {
			/* read data from stdion */
			memset(buffer, 0, BUFLEN);
			fgets(buffer, BUFLEN - 1, stdin);

			/* if 'exit' command was sent, stop communication with the server */
			if (strncmp(buffer, EXIT, 4) == 0) {
				break;
			} else if (strncmp(buffer, SUBSC, strlen(SUBSC)) == 0){
				/* if subscriber received a 'subscribe' comman,
				 * send to server a struct of type 'Action' 
				 * communicating the updates */
				strcpy(action.type, SUBSC);
				char *aux = strtok(buffer, SEP); /*'subscribe'*/
				aux = strtok(NULL, SEP); 		 /*topic*/
				strcpy(action.topic, aux);
				aux = strtok(NULL, SEP);		 /*sf*/
				action.sf = atoi(aux);
				fprintf(stdout, "%s", SUBSCRIBED);
			} else if (strncmp(buffer, UNSUBSC, strlen(UNSUBSC)) == 0){
				strcpy(action.type, UNSUBSC);
				char *aux = strtok(buffer, SEP);
				aux = strtok(NULL, SEP);
				strncpy(action.topic, aux, 50);
				action.sf = 2; 					/* something irrelevant */
				fprintf(stdout, "%s", UNSUBSCRIBED);
			}
			/* send action to server */
			ret = send(sockfd, (char*)&action, sizeof(Action), 0);
			DIE(ret < 0, "Failed to send action to server.\n");
		} else if(FD_ISSET(sockfd, &tmp_fds)){
			/* subscriber received a message from server */
			/* get size of the package */
			memset(tcp_buff, 0, TCP_SIZE+1);
			ret = recv(sockfd, tcp_buff, 10, 0);
			DIE(ret < 0, "Failed to received size of package from server.\n");
            if(ret == 0)
				break;
                
			int dim = atoi(tcp_buff);      		/* convert the size */
			memset(tcp_buff, 0, TCP_SIZE+1);	/* clear buffer */
			/* receive the actual tcp packet */
			int received = 0;
			while(received < dim){
				ret = recv(sockfd, tcp_buff, dim, 0);
				DIE(ret < 0, "Failed to receive package from server.\n");
				if(ret == 0)
					break;
				
				received += ret;
			}
			/* cast the tcp packet to a TCP_Packet struct */
			tcp_packet = (TCP_Packet *)tcp_buff;

			fprintf(stdout, "%s:%hu - %s - %s - %s\n", tcp_packet->ip, tcp_packet->port,
				tcp_packet->topic, tcp_packet->type, tcp_packet->payload);
        }
	}
	/* close socket for subscriber */
	close(sockfd);

	return 0;
}
