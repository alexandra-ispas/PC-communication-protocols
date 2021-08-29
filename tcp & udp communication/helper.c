#include "helper.h"

TCP_Packet *udp_to_tcp(UDP_Packet* udp_packet, struct sockaddr_in udp_addr){
	DIE(udp_packet->type > 3, "Incorrect UDP packet type.\n");

	long long int_aux;
	double real_aux = 0;
	/* allocate memory for the new tcp packet */
	TCP_Packet* tcp_packet = (TCP_Packet*)calloc(1, sizeof(TCP_Packet));
	DIE(tcp_packet == NULL, "Failed to allocate memory.\n");
	strcpy(tcp_packet->ip, inet_ntoa(udp_addr.sin_addr));
	tcp_packet->port = ntohs(udp_addr.sin_port);
	strncpy(tcp_packet->topic, udp_packet->topic, 50);
	tcp_packet->topic[50] = 0;

	switch (udp_packet->type){

		case 0:	 
			/* an int was received 
				* so the first byte from the payload should be a sign byte*/
			DIE(udp_packet->payload[0] > 1, "Incorrect sign byte from UDP packet.\n");
			strcpy(tcp_packet->type, INT);
			if(udp_packet->payload[0]){
				/* the number is negative */
				int_aux = -1;
			} else {
				/*the number is positive */
				int_aux = 1;
			}
			/* convert the number, eliminating the sign byte from payload */
			int_aux *= ntohl(*(uint32_t*)(udp_packet->payload + 1));
			sprintf(tcp_packet->payload, "%lld", int_aux);
			break;
		case 1:
			/* a short number was received */
			strcpy(tcp_packet->type, SHORT);
			/* get the actual number */
			real_aux = 1.0 * (ntohs(*(uint16_t*)(udp_packet->payload))) / 100;
			sprintf(tcp_packet->payload, "%.2f", real_aux);
			break;

		case 2:
			/* a float number was received */
			DIE(udp_packet->payload[0] > 1, "Incorrect sign byte from UDP packet.\n");
			strcpy(tcp_packet->type, FLOAT);
			/* get the actual number */
			if(udp_packet->payload[0]){
				real_aux = (-1.0);
			} else {
				real_aux = 1.0;
			}
			real_aux *= (ntohl(*(uint32_t*)(udp_packet->payload + 1)));
			real_aux /= pow(10, udp_packet->payload[5]);
			sprintf(tcp_packet->payload, "%lf", real_aux);
			break;
		default:
			/* a string was received */
			strcpy(tcp_packet->type, STRING);
			/* just put the received string in the tcp payload */
			strcpy(tcp_packet->payload, udp_packet->payload);
			break;
	}

	sprintf(tcp_packet->size, "%ld", sizeof(TCP_Packet) - MAX_LEN + 
									strlen(tcp_packet->payload));
    return tcp_packet;
}

Client* get_client(Subscribers *subscribers, char *buffer){
    for(int j = 0; j < subscribers->size; j++){
        /* there is a client with the same ID */
        if(strcmp(subscribers->clients[j]->id, buffer) == 0){
            return subscribers->clients[j];
        }
    }
    return NULL;
}

void send_tcp(Subscribers *subscribers, TCP_Packet* tcp_packet){
	for(int j = 0; j < subscribers->size; j++){
		for(int k = 0; k < subscribers->clients[j]->topic_no; k++){
			if(strcmp(subscribers->clients[j]->topics[k]->topic, 
											tcp_packet->topic) == 0){
				/* if there is a disconnected client with the same
					* topic and sf = 1, stores the packet */
				if(subscribers->clients[j]->connected == 0 &&
					subscribers->clients[j]->topics[k]->sf == 1){
						/* reallocate memory for client tcp packages' array */
						if(subscribers->clients[j]->topics[k]->tcp_no == 
							subscribers->clients[j]->topics[k]->tcp_capacity){
								/*double the capacity */
								subscribers->clients[j]->topics[k]->tcp_capacity *= 2;

								subscribers->clients[j]->topics[k]->tcps = 
									(TCP_Packet**) realloc(subscribers->clients[j]->topics[k]->tcps,
									subscribers->clients[j]->topics[k]->tcp_capacity * 
																	sizeof(TCP_Packet*));
								DIE(subscribers->clients[j]->topics[k]->tcps == NULL,
									"Failed to allocate memory.\n");
						}
						/* add the packet to the client's array 
							* and increase the number ot tcp packages for client */
						int nr = subscribers->clients[j]->topics[k]->tcp_no++;
						subscribers->clients[j]->topics[k]->tcps[nr] = tcp_packet;
						break;  /* the client cannot subscribe to the same topic
									* twice so we should stop iterating over the
									* topics if we have already found the one */
				} else if (subscribers->clients[j]->connected == 1) {
					/* if the subscriber with this topic is connected,
						* sent the size of the package followed by the tcp package */
					send(subscribers->clients[j]->socket, tcp_packet->size, 10, 0);
					send(subscribers->clients[j]->socket, (char*)tcp_packet, 
												atoi(tcp_packet->size), 0);
					break;
				}	
			}
		}
	}
}

void disconnect_subscriber(Subscribers *subscribers, int socket){
    for(int k = 0; k < subscribers->size; k++){		
        if(subscribers->clients[k]->socket == socket){
            printf("Client %s disconnected.\n", subscribers->clients[k]->id);
            /* disconnect the client */
            subscribers->clients[k]->connected = 0;
            for(int j = 0; j < subscribers->clients[k]->topic_no; j++){
                if(subscribers->clients[k]->topics[j]->sf == 1){
                    /* if the client has sf = 1 for a specific topic,
                        * allocate memory for the tcp packages array 
                        * because data will be stored there */
                    subscribers->clients[k]->topics[j]->tcps = 
                        (TCP_Packet**)calloc(10, sizeof(TCP_Packet*));
                    DIE(subscribers->clients[k]->topics[j]->tcps == NULL, 
                                        "Failed to allocate memory.\n");
                    subscribers->clients[k]->topics[j]->tcp_no = 0;
                    subscribers->clients[k]->topics[j]->tcp_capacity = 10;
                }
            }
            /* we found our client, there is no need to iterate over the array */
            break;
        }
    }
    /* close socket */
    close(socket);
}

void subscribe_to_topic(Subscribers *subscribers, Action *action, int socket){
    /* a new topic is created for this client */
    Topic_packet *new_topic = (Topic_packet*)calloc(1, sizeof(Topic_packet));
    DIE(new_topic == NULL, "Failed to allocate memory.\n");
    /* initialize the values of the topic based on the 
        * 'Action' structure received */
    new_topic->sf = action->sf;
    strcpy(new_topic->topic, action->topic);
    /* no tcp packets are currently stored, 
        * the array will be allocated when the client disconnects */
    new_topic->tcps = NULL;
    new_topic->tcp_no = 0;

    for(int k = 0; k < subscribers->size; k++){
        if(subscribers->clients[k]->socket == socket){
            /* reallocate memory for the array of topics */
            if(subscribers->clients[k]->topic_capacity == 
                        subscribers->clients[k]->topic_no){
                /* double the capacity */
                subscribers->clients[k]->topic_capacity *= 2;
                subscribers->clients[k]->topics = 
                    (Topic_packet**) realloc(subscribers->clients[k]->topics, 
                    subscribers->clients[k]->topic_capacity * sizeof(Topic_packet*));
                DIE(subscribers->clients[k]->topics == NULL, "Failed to allocate memory.\n");
            }
            /* add topic to client */
            subscribers->clients[k]->topics[subscribers->clients[k]->topic_no++] = new_topic;
            break;
        }
    }
}

Subscribers *create_subscribers(){
	Subscribers *subscribers = (Subscribers *)calloc(1, sizeof(Subscribers));
	DIE(subscribers == NULL, "Failed to allocate memory.\n");

	/* initialize fields for struct */
	subscribers->capacity = 10;
	subscribers->size = 0;
	subscribers->clients = (Client**)calloc(subscribers->capacity, sizeof(Client*));

    return subscribers;
}

void add_new_client(Subscribers *subscribers, char *buffer, int newtcp_socket){
    Client *new_subscriber = (Client *)calloc(1, sizeof(Client));
    DIE(new_subscriber == NULL, "Failed to allocate memory.\n");

    /* initialize fields for the new subscriber */
    strcpy(new_subscriber->id, buffer);
    new_subscriber->connected = 1;
    new_subscriber->socket = newtcp_socket;
    new_subscriber->topic_no = 0;
    new_subscriber->topic_capacity = 20;

    new_subscriber->topics = (Topic_packet**)calloc(new_subscriber->topic_capacity, 
                                                sizeof(Topic_packet*));
    DIE(new_subscriber->topics == NULL, "Failed to allocate memory.\n");

    /* reallocate memory for the array of clients */
    if(subscribers->capacity == subscribers->size){
        /* double the capacity */
        subscribers->capacity *=2;
        subscribers->clients = (Client **)realloc(subscribers->clients, 
            subscribers->capacity * sizeof(Client*));
        DIE(subscribers->clients == NULL, "Failed to reallocate memory.\n");
    }
    /* add the new subscriber in the array */
    subscribers->clients[subscribers->size++] = new_subscriber;
}

void reconnect_an_old_client(Client *client, int newtcp_socket){
    /*update the subscriber's socket */
    client->socket = newtcp_socket;
    /* all the packages which have had been received 
        * from the udp clients while the subscriber
        * was disabled are sent now */
    for(int t = 0; t < client->topic_no; t++){
        for(int p = 0; p < client->topics[t]->tcp_no; p++){
            TCP_Packet *pkt = client->topics[t]->tcps[p];
            /* send the size of the packet first, 
                * then the actual tcp packet */
            send(newtcp_socket, pkt->size, 10, 0);
            send(newtcp_socket, (char*)pkt, atoi(pkt->size), 0);
        }
    }
    /* the client is reconnected */
    client->connected = 1;
    /* free memory for all the tcp packages which have
        * been stored */
    for(int t = 0; t < client->topic_no; t++){
        for(int p = 0; p < client->topics[t]->tcp_no; p++){
            free(client->topics[t]->tcps[p]);
        }
        free(client->topics[t]->tcps);
        client->topics[t]->tcp_no = 0;
        client->topics[t]->tcp_capacity = 0;
    }
}

void close_sockets(int fdmax, fd_set read_fds){
	for(int i = 2; i<= fdmax; i++){
		if(FD_ISSET(i, &read_fds)){
			close(i);
		}
	}
}

void free_memory(Subscribers* subscribers){
	for(int i = 0; i < subscribers->size; i++){
		for(int j = 0; j < subscribers->clients[i]->topic_no; j++){
			for(int k = 0; k < subscribers->clients[i]->topics[j]->tcp_no; k++){
				free(subscribers->clients[i]->topics[j]->tcps[k]);
			}
			free(subscribers->clients[i]->topics[j]->tcps);
			free(subscribers->clients[i]->topics[j]);
		}
		free(subscribers->clients[i]->topics);
		free(subscribers->clients[i]);
	}
	free(subscribers->clients);
	free(subscribers);
}
