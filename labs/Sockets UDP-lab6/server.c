/*
*  	Protocoale de comunicatii: 
*  	Laborator 6: UDP
*	mini-server de backup fisiere
*/

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
	fprintf(stderr,"Usage: %s server_port file\n",file);
	exit(0);
}

/*
*	Utilizare: ./server server_port
*/
int main(int argc,char**argv)
{
	int fd;
	
	struct sockaddr_in my_sockaddr, from_station ;
	char buf[BUFLEN];


	/*Deschidere socket*/
	int socky = socket(AF_INET, SOCK_DGRAM, 0);

	if (socky == -1) {
		fprintf(stdout, "Fail to open socket!");
		exit(EXIT_FAILURE);
	}
	
	/*Setare struct sockaddr_in pentru a asculta pe portul respectiv */
	from_station.sin_family = AF_INET;
	from_station.sin_port = htons(atoi(argv[1]));
	from_station.sin_addr.s_addr = INADDR_ANY;
	socklen_t st = sizeof(from_station);
	
	/* Legare proprietati de socket */
	if (bind(socky, (struct sockaddr *) &(from_station), st) == -1) {
		fprintf(stderr, "eroare");
		exit(EXIT_FAILURE);
	}
	
	recvfrom(socky, buf, BUFLEN, 0, (struct sockaddr *) &(from_station), &st);
	int numFiles = atoi(buf);
	printf("Numar fisiere trimise: %d\n", numFiles);

	for (int i = 0; i < numFiles; i++) {
		int dim = recvfrom(socky, buf, BUFLEN, 0, (struct sockaddr *) &(from_station), &st);

		char numeF[BUFLEN];
		char nF[BUFLEN];
		strncpy(nF, buf, dim);
		printf("Nume fisier trimis: %s; ", nF);
		sprintf(numeF, "server_%s", nF + 7);

		/* Deschidere fisier pentru scriere */
		DIE((fd=open(numeF,O_WRONLY|O_CREAT|O_TRUNC,0644))==-1,"open file");
		printf("Nume fisier creat: %s\n", numeF);

		while (1) {
			dim = recvfrom(socky, buf, BUFLEN, 0, (struct sockaddr *) &(from_station), &st);
			if (strncmp (buf, "Gata boss", 7) == 0) {
				break;
			}
			write(fd, buf, dim);
		}

		/*Inchidere fisier*/
		close(fd);
	}
	/*Inchidere socket*/	
	close(socky);

	return 0;
}
