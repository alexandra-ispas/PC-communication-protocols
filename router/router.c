#include "helper.h"

int main(int argc, char *argv[]){
	packet m;
	int rc;

	init(argc - 2, argv + 2);

	struct route_table_entry **rtable;
	struct in_addr* router = (struct in_addr*)malloc(sizeof(struct in_addr));
	uint32_t router_ip_address;
	struct arp_table *arp_table = create_arp_table();
	uint8_t router_mac[6];
	queue q = queue_create ();
	
	read_rtable(&rtable, argv[1]);

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header* eth_hdr = (struct ether_header*)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		//router mac address on that interface
		get_interface_mac(m.interface, router_mac);
		//router ip address on that interface
		inet_aton(get_interface_ip(m.interface), router);
		router_ip_address = router->s_addr;

		struct arp_header* arp_hdr = parse_arp(m.payload);
		if(arp_hdr != NULL){
			//the router received an ARPOP_REQUEST
			if(htons(arp_hdr->op) == ARPOP_REQUEST 
				&& arp_hdr->tpa == router_ip_address){

				received_arp_request(arp_hdr, router_mac, router_ip_address, m);
				continue;
			}
			//the router received an ARPOP_REPLY
			else if(htons(arp_hdr->op) == ARPOP_REPLY){
				received_arp_reply(q, arp_table, arp_hdr);
				continue;
			}
		}
	
		struct icmphdr *icmp_hdr = parse_icmp(m.payload);
		//the router received an ECHO REQUEST
		if(icmp_hdr != NULL && icmp_hdr->type == ICMP_ECHO &&
				ip_hdr->daddr == router_ip_address){
			//send ECHO REPLY
			send_icmp(ip_hdr->saddr, router_ip_address, eth_hdr->ether_dhost, 
				eth_hdr->ether_shost, ICMP_ECHOREPLY, 0, m.interface, getpid(), 
				getpid());
			continue;
		}	

		if(htons(eth_hdr->ether_type) == ETHERTYPE_IP){
			//check if packet is correct
			struct route_table_entry *route = 
				check_packet(&ip_hdr, m, router_ip_address, eth_hdr->ether_dhost, 
							rtable);

			//forward the packet or send an ARP request if the MAC
			//address of the next hop is not known
			forward(route, m, arp_table, &q);
		}
	}
	free(q);
	free(router);
	free_arp_table(arp_table);
	free_routing_table(rtable);
	return 0;
}
