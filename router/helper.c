#include "helper.h"

void read_rtable(struct route_table_entry ***rtable, char* table){
	FILE *in = fopen(table, "rt");
	if(in == NULL){
		printf("Failed to open routing table\n");
		exit(EXIT_FAILURE);
	}

	(*rtable) = (struct route_table_entry**)calloc(N, sizeof(struct route_table_entry*));
	if(rtable == NULL){
		printf("Filed to allocate memory\n");
		exit(EXIT_FAILURE);
	}

	char buffer[100];
	char *s1, *s2, *s3, *s4;
	int idx = 0;
	struct in_addr* aux = (struct in_addr*)malloc(sizeof(struct in_addr));
	while(fgets(buffer, 100, in) != NULL){
		struct route_table_entry *r = (struct route_table_entry*)calloc(1, sizeof(struct route_table_entry));
		if(r == NULL){
            printf("Filed to allocate memory\n");
            for(int j = 0; j < idx; j++)
                free( (*rtable)[j]);
            free((*rtable));
		    exit(EXIT_FAILURE);
        }
		s1 = strtok(buffer, " ");
		inet_aton(s1, aux);
		r->prefix =aux->s_addr;

		s2 = strtok(NULL, " ");
		inet_aton(s2, aux);
		r->next_hop = aux->s_addr;
		
		s3 = strtok(NULL, " ");
		inet_aton(s3, aux);
		r->mask = aux->s_addr;
		
		s4 = strtok(NULL, " ");
		r->interface = atoi(s4);
		(*rtable)[idx++] = r;
	}
	fclose(in);

	//sort the routing table
	qsort((*rtable), N, sizeof((*rtable)[0]), cmp);	

	if((*rtable) == NULL){
		fprintf(stdout, "Error while reading routing table.\n");
	}
	fprintf(stdout, "Finished reading table.\n");
}


void free_routing_table(struct route_table_entry **rtable){
	for(int i = 0; i < N; i++)
		free(rtable[i]);
	free(rtable);
}

struct arp_table *create_arp_table(){
	struct arp_table *arp_table = (struct arp_table *)calloc(1, sizeof(struct arp_table *));
	if(arp_table == NULL){
		fprintf(stdout, "Allocation error\n");
		exit(EXIT_FAILURE);
	}
	arp_table->size = 0;
	arp_table->capacity = 0;
	return arp_table;
}

struct route_table_entry *get_best_route(struct route_table_entry **rtable,
									 uint32_t dest_ip) {

	int l = 0, r = N - 1;
	while (1) {
		if(l > r)
			break;

		uint32_t m = l + ((r - l) / 2);
		uint32_t prefix = dest_ip & rtable[m]->mask;
		if (prefix == rtable[m]->prefix){
			while (1){
				//get longest prefix
				if( (m <= 0) ||
					(rtable[m]->prefix != rtable[m-1]->prefix)||
					(rtable[m]->mask >= rtable[m-1]->mask))
						break;
				m--;
			}
			return rtable[m];
		}
		else if (prefix < rtable[m]->prefix)
			r = m - 1;
		else
			l = m + 1;
	}
	return NULL;
}

struct arp_entry *get_arp_route(struct arp_table *atable, uint32_t dest_ip) {
	for(int i = 0; i < atable->size; i++)
		if(atable->entries[i]->ip == dest_ip)
			return atable->entries[i];
	return NULL;
}

void  add_arp_entry(struct arp_table **arp_table, struct arp_entry *arp_entry){
	//if the table is full, reallocate memory
	if((*arp_table)->size == (*arp_table)->capacity){
		(*arp_table)->capacity *= 2;
		(*arp_table)->entries = realloc((*arp_table)->entries, (*arp_table)->capacity);
		if((*arp_table)->entries == NULL){
			fprintf(stdout, "Realloc failed.\n");
			exit(EXIT_FAILURE);
		}
	}
	(*arp_table)->entries[(*arp_table)->size++] = arp_entry;
}

struct arp_entry *build_arp_entry(uint32_t ip, uint8_t mac[6]){
	//allocate the new row in the ARP table
	struct arp_entry *entry = (struct arp_entry*)calloc(1, sizeof(struct arp_entry));
	if(entry == NULL){
		fprintf(stdout, "Allocation error\n");
		exit(EXIT_FAILURE);
	}
	entry->ip = ip;
	memcpy(entry->mac, mac, 6);
	return entry;
}

void free_arp_table(struct arp_table *arp_table){
	for(int i = 0; i < arp_table->size; i++)
		free(arp_table->entries[i]);
	free(arp_table->entries);
	arp_table->size = 0; 
	arp_table->capacity = 0;
	free(arp_table);
}

void received_arp_request(struct arp_header* arp_hdr, uint8_t router_mac[6], 
						uint32_t router_ip_address, packet m){
				
	struct ether_header* eth_hdr = (struct ether_header*)m.payload;
	
	memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, 6);
	memcpy(eth_hdr->ether_shost, router_mac, 6);
	send_arp(arp_hdr->spa, router_ip_address, eth_hdr, m.interface, ntohs(ARPOP_REPLY));					
}

int cmp (const void *a, const void *b){
	struct route_table_entry* a1 = *(struct route_table_entry**)a;
	struct route_table_entry* b1 = *(struct route_table_entry**)b;

	if(a1->prefix != b1->prefix)
		return (a1->prefix - b1->prefix);
	else 
		return (a1->mask - b1->mask);
}

void received_arp_reply(queue q, struct arp_table *arp_table, struct arp_header *arp_hdr){
	//if the queue is empty, an ARP reply has already been received
	//the packet is dropped
	if(queue_empty(q))
		return;

	struct arp_entry *arp_entry = get_arp_route(arp_table, arp_hdr->spa);
	if(arp_entry == NULL){
		//if there is no entry in the ARP table for this IP address.
		//I create one
		struct arp_entry *new_entry = build_arp_entry(arp_hdr->spa, arp_hdr->sha);
		add_arp_entry(&arp_table, new_entry);
	}
	//get the first packet from the queue and forward it
	packet front = *(packet*)queue_deq(q);
	struct ether_header* eth_hdr = (struct ether_header*)front.payload;
	
	build_ethhdr(eth_hdr, arp_hdr->tha, arp_hdr->sha, htons(ETHERTYPE_IP));
	send_packet(front.interface, &front);
}

struct route_table_entry *check_packet(struct iphdr **ip_hdr, packet m,  
								uint32_t router_ip_address,	uint8_t router_mac[6], 
								struct route_table_entry **rtable){

	struct ether_header* eth_hdr = (struct ether_header*)m.payload;

	//check ttl
	if((*ip_hdr)->ttl <= 1){
		//wrong ttl, drop the packet
		send_icmp_error((*ip_hdr)->saddr, router_ip_address, eth_hdr->ether_dhost, 
			router_mac, ICMP_TIME_EXCEEDED, ICMP_EXC_TTL, m.interface);
		return NULL;
	}
	//check the checksum
	if(ip_checksum((*ip_hdr), sizeof(struct iphdr)) != 0){
		//wrong checksum, drop the packet
		return NULL;
	}

	//update ttl and checksum
	(*ip_hdr)->ttl--;
	(*ip_hdr)->check = 0;
	(*ip_hdr)->check = ip_checksum((*ip_hdr), sizeof(struct iphdr));

	//check next hop
	struct route_table_entry *route = get_best_route(rtable,(*ip_hdr)->daddr);
	if(route == NULL){
		//destination unreachable
		send_icmp_error((*ip_hdr)->saddr, router_ip_address, eth_hdr->ether_dhost, 
					router_mac, ICMP_DEST_UNREACH, ICMP_NET_UNREACH, m.interface);
		return NULL;
	}
	//return routing table entry with next hop
	return route;
}

void forward(struct route_table_entry *route, packet m, struct arp_table *arp_table,
										queue *q){

	struct ether_header* eth_hdr = (struct ether_header*)m.payload;
	uint8_t broadcast[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct in_addr* router = (struct in_addr*)malloc(sizeof(struct in_addr));

	//create a copy of the packet
	packet aux;
	aux.interface = route->interface;
	memcpy(aux.payload, m.payload, m.len);
	aux.len = m.len;

	get_interface_mac(route->interface, eth_hdr->ether_shost);
	inet_aton(get_interface_ip(route->interface), router);
	uint32_t router_ip_address = router->s_addr;
	free(router);

	//there is no entry in the ARP table for the next hop
	struct arp_entry *destination = get_arp_route(arp_table, route->next_hop);
	if(destination == NULL){
		build_ethhdr(eth_hdr, eth_hdr->ether_shost, broadcast, htons(ETHERTYPE_ARP));
		//add the copy of the packet to the queue
		queue_enq((*q), &aux);
		//send ARP request
		send_arp(route->next_hop, router_ip_address, eth_hdr, route->interface, 
						htons(ARPOP_REQUEST));
		return;
	}
	//the MAC address of the next hop is known so the packet is forwarded
	memcpy(eth_hdr->ether_dhost, destination->mac, 6);
	send_packet(route->interface, &m);
}
