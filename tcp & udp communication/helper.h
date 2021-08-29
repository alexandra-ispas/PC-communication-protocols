#ifndef _HELPERS_H
#define _HELPERS_H 1
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <math.h>
#include <limits.h>

/*
 * Macro din laborator
 */

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define BUFLEN		1552	/* maximim size for data */
#define MAX_CLIENTS	10		/* maximum number of waiting clients */
#define TOPIC_SIZE 	51
#define TCP_SIZE sizeof(TCP_Packet) + 1		/* maximum size of a tcp packet */

#define MAX_LEN 1501		/* maximim payload size */
#define TCP_LEN sizeof(struct sockaddr)
#define UDP_LEN sizeof(struct sockaddr)
#define SEP " "				/* used for 'strtok' */
#define SUBSC "subscribe"
#define UNSUBSC "unsubscribe"
#define EXIT "exit\n" 
#define SERVER 0
#define INT "INT"
#define SHORT "SHORT_REAL"
#define FLOAT "FLOAT"
#define STRING "STRING"
#define SUBSCRIBED "Subscribed to topic.\n"
#define UNSUBSCRIBED "Unsubscribed from topic.\n"

typedef struct tcp_packet{
	char size[10];
	char ip[16];
	uint16_t port;
	char type[11];
	char topic[TOPIC_SIZE];
	char payload[MAX_LEN];
}__attribute__((packed))TCP_Packet;

typedef struct topic {
	uint8_t sf;
	char topic[TOPIC_SIZE];
	TCP_Packet **tcps;
	int tcp_no;
	int tcp_capacity;
}__attribute__((packed))Topic_packet;

typedef struct client {
	char id[11];
	uint8_t connected;
	int socket;
	int topic_no;
	int topic_capacity;
	Topic_packet **topics;
}__attribute__((packed))Client;

typedef struct subscribers{
	int size;
	int capacity;
	Client **clients;
}__attribute__((packed))Subscribers;

typedef struct udp_packet {
	char topic[50];
	uint8_t type;			
	char payload[MAX_LEN];
}__attribute__((packed))UDP_Packet;

typedef struct action {
	char type[20]; 				/*"subscribe"/"unsubscribe"*/
	char topic[TOPIC_SIZE];
	unsigned char sf;
}__attribute__((packed))Action;

/* allocate memory for a struct of type 'Subscribers' */
Subscribers *create_subscribers();

/* returns a TCP packet after receiving a UDP message */
TCP_Packet *udp_to_tcp(UDP_Packet* udp_packet, struct sockaddr_in udp_addr);

void free_memory(Subscribers* subscribers);

/* return a client who has the given ID or NULL */
Client* get_client(Subscribers *subscribers, char *buffer);

/* send the tcp packet to the connected clients
 * or store it for the disconnected cliens who have sf = 1 */
void send_tcp(Subscribers *subscribers, TCP_Packet* tcp_packet);

/* disconnect a subscriber because it stopped the communication */
void disconnect_subscriber(Subscribers *subscribers, int socket);

/* a cliend subscribed to a topic and the sever updates its data */
void subscribe_to_topic(Subscribers *subscribers, Action *action, int socket);

/* a new client was connected to the server */
void add_new_client(Subscribers *subscribers, char *buffer, int newtcp_socket);

/* server receives a connection from ab old client */
void reconnect_an_old_client(Client *client, int newtcp_socket);

/* close all the subcbribers */
void close_sockets(int fdmax, fd_set read_fds);
#endif
