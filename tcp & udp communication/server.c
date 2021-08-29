#include "helper.h"

void usage(char *file){
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[])
{
	/* disable buffering */
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	/* check if program has enough input arguments */
	if (argc < 2) {
		usage(argv[0]);
	}
	int newtcp_socket, flag = 1, stop = 0;
	char buffer[BUFLEN];
	struct sockaddr_in tcp_addr, udp_addr, subscr_addr;
	int ret;  	/* used for storing the exit codes from functions */
	fd_set read_fds;	/* set used in 'select()' */
	fd_set tmp_fds;		/* temporary set */
	socklen_t udp_len = UDP_LEN;
	socklen_t tcp_len = TCP_LEN;

	/* struct used for storing the subscribers of the server */
	Subscribers *subscribers = create_subscribers();
	TCP_Packet *tcp_packet;
	UDP_Packet *udp_packet;
	Action *action;

	/* open tcp socket */
	int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
	DIE(tcp_socket < 0, "Failed to create TCP socket.\n");

	/* open udp socket */
	int udp_socket = socket(PF_INET, SOCK_DGRAM, 0);
	DIE(udp_socket < 0, "Failed to create UDP socket.\n");

	/* initialize data for both sockets */
	memset((char *) &tcp_addr, 0, sizeof(tcp_addr));
	tcp_addr.sin_family = AF_INET;
	tcp_addr.sin_port = htons(atoi(argv[1]));
	tcp_addr.sin_addr.s_addr = INADDR_ANY;

	memset((char *) &udp_addr, 0, sizeof(udp_addr));
	udp_addr.sin_family = AF_INET;
	udp_addr.sin_port = htons(atoi(argv[1]));
	udp_addr.sin_addr.s_addr = INADDR_ANY;

	/* bind tcp socket */
	ret = bind(tcp_socket, (struct sockaddr *) &tcp_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "Failed to bind TCP socket.\n");
	/* bind udp socket */
	ret = bind(udp_socket, (struct sockaddr *) &udp_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "Failed to bind UDP socket.\n");
	ret = listen(tcp_socket, MAX_CLIENTS);
	DIE(ret < 0, "Failed to listen to TCP socket.\n");

	/* empty 'read_fds' set and 'temp_fds' set */
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	/* add tcp and udp sockets into the read_fds set */
	FD_SET(tcp_socket, &read_fds);
	FD_SET(udp_socket, &read_fds);
	FD_SET(0, &read_fds);
	int fdmax = tcp_socket;

	while (stop == 0) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "Failed select a socket.\n");
		/* look for the sockets whitch sends data */
		for (int i = 0; i <= fdmax && stop == 0; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				/* the socket is of tcp type
				 * so we will receive a tcp packet */
				if (i == tcp_socket) {
					newtcp_socket = accept(tcp_socket, (struct sockaddr *) &subscr_addr, &tcp_len);
					DIE(newtcp_socket < 0, "Failed to accept subscriber.\n");
					/* disabe Nagle's algorithm for this socket */
                    setsockopt(newtcp_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
					/* add the new socket to the reading sockets' list */
					FD_SET(newtcp_socket, &read_fds);
					/* update fdmax */
					if (newtcp_socket > fdmax) { 
						fdmax = newtcp_socket;
					}
					/* receive subscriber's ID */
					memset(buffer, 0, BUFLEN);
					ret = recv(newtcp_socket, buffer, BUFLEN, 0);
					DIE(ret < 0, "Failed to receive ID.\n");

					/* check if there already is a client with this ID */
					Client *client = get_client(subscribers, buffer);
					if(client == NULL){
						/* this client doesn't exist
						 * we have to create a new subscriber */
						add_new_client(subscribers, buffer, newtcp_socket);
						fprintf(stdout, "New client %s connected from %s:%d\n", buffer, 
								inet_ntoa(subscr_addr.sin_addr), ntohs(subscr_addr.sin_port));
						/* there is noting else to do for this socket now*/
						continue;
					}
					if(client->connected == 1){
						/* the client is already connected */
						/* empty the new socket */	
						FD_CLR(newtcp_socket, &read_fds);
						/* pick the last fdmax, 
						* because the current socket will be closed */
						for (int k = fdmax; k > 2; --k) {
							if(FD_ISSET(k, &read_fds)) {
								fdmax = k;
								break;
							}
						}
						/* close the new socket */
						close(newtcp_socket);
						fprintf(stdout, "Client %s already connected.\n", buffer);
						continue;
					} else if(client->connected == 0){
						/* the current subscriber is an old clinet reconnecting */
						reconnect_an_old_client(client, newtcp_socket);
						fprintf(stdout, "New client %s connected from %s:%d\n", buffer, 
							inet_ntoa(subscr_addr.sin_addr), ntohs(subscr_addr.sin_port));
						continue;
					}
				} else if (i == udp_socket){
					/* reaceived a message from a udp client */
					memset(buffer, 0, BUFLEN);
					ret = recvfrom(udp_socket, buffer, BUFLEN, 0, (struct sockaddr*) &udp_addr, &udp_len);
					DIE(ret < 0, "Failed to received something on the UDP socket.\n");
					/* cast the message to a UDP_Packet struct */
					udp_packet = (UDP_Packet*)buffer;
					/* create the tcp packet from the udp message */
					tcp_packet = udp_to_tcp(udp_packet, udp_addr);
					send_tcp(subscribers, tcp_packet);
					continue;
				} else if(i == SERVER) {
					/* if we received a message from the server,
					 * it can only be 'exit' and stop the server read the message */
					memset(buffer, 0, BUFLEN);
					fgets(buffer, BUFLEN-1, stdin);
					if(strcmp(buffer, EXIT) == 0){
						/* stop the programme */
						stop = 1;
					} else {
						fprintf(stdout, "Can only accept 'exit' command.\n");
						continue;
					}
				} else {
					/* sever received a tcp message from a subscriber */
					memset(buffer, 0, BUFLEN);
					ret = recv(i, buffer, sizeof(Action), 0);
					DIE(ret < 0, "Failed to receive message from subscriber.\n)");
					if(ret == 0){		
						/* the subscriber stopped the communication.
						 * The client will be disconnected and the socket closed */
						disconnect_subscriber(subscribers, i);
						/* empty the current socket and update fdmax to the last greatest value*/
						FD_CLR(i, &read_fds);
						for (int j = fdmax; j > 2; --j) {
							if(FD_ISSET(j, &read_fds)) {
								fdmax = j;
								break;
							}
						}
						continue;
					} else {	
						/* there is a message received from the subscriber*/
						action = (Action *) buffer;
						/* if the message is an 'unsubscribe' one */
						if(strcmp(action->type, UNSUBSC) == 0){
							int ok = 0; /* varianble used for exiting two loops simultaneously */
							for(int k = 0; k < subscribers->size && ok == 0; k++){
								if(subscribers->clients[k]->socket == i){
									/* we found our client */
									for(int j = 0; j < subscribers->clients[k]->topic_no && ok == 0; j++){
										if(strcmp(subscribers->clients[k]->topics[j]->topic, action->topic) == 0){
											/* we found the topic from which we want to unsubscribe */
											strcpy(subscribers->clients[k]->topics[j]->topic, "");
											ok = 1;
										}
									}
								}
							}
							continue;
						} else {
							/* if the message is an 'subscribe' one */
							subscribe_to_topic(subscribers, action, i);
							continue;
						}
					}	
				}
			}
		}
	}
	/* free memory and close sockets*/
	free_memory(subscribers);
	close_sockets(fdmax, read_fds);
	return 0;
}
