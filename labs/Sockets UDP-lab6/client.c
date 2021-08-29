#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>

#include "helpers.h"

void usage(char*file)
{
	fprintf(stderr,"Usage: %s ip_server port_server file\n",file);
	exit(0);   
}

/*
*	Utilizare: ./client ip_server port_server nume_fisier_trimis
*/
int main(int argc,char**argv)
{
	int fd;
	struct sockaddr_in to_station;
	char buf[BUFLEN];

	/*Deschidere socket*/
	int socky = socket(AF_INET, SOCK_DGRAM, 0);
	
	/*Setare struct sockaddr_in pentru a specifica unde trimit datele*/
	to_station.sin_family = AF_INET;
	to_station.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &(to_station.sin_addr));

	sprintf(buf, "%d", argc - 3);
	sendto(socky, buf, strlen(buf), 0, (struct sockaddr *) &(to_station), sizeof(to_station));

	int dim;
	for (int i = 3; i < argc; i++) {
		DIE((fd=open(argv[i],O_RDONLY))==-1,"open file");
		sprintf(buf, "%s", argv[i]);
		sendto(socky, buf, strlen(buf), 0, (struct sockaddr *) &(to_station), sizeof(to_station));

		while ((dim = read(fd, buf, BUFLEN)) > 0) {
			sendto(socky, buf, dim, 0, (struct sockaddr *) &(to_station), sizeof(to_station));
		}
		
		sprintf(buf, "Gata boss");
		sendto(socky, buf, strlen("Gata boss"), 0, (struct sockaddr *) &(to_station), sizeof(to_station));

		/*Inchidere fisier*/
		close(fd);
	}
	/*Inchidere socket*/
	close(socky);
	
	return 0;
}
