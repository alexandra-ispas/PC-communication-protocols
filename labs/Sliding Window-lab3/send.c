#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

void check(msg *t){

	if (recv_message(t)<0)
	   perror("Receive error ...");
	else 
	 printf("[send] Got reply\n");
}

//return the checksum of a message
int get_checksum(msg t){
	int sum = 0;

	for(int i = 0; i < MSGSIZE; i++){
		for(int j = 0; j < 8; j++){
			sum ^= (1 << j) & t.payload[i];
		}
	}
	return sum;
}

void send_file(char* file, msg t, int count){

	int fp = open(file, O_RDONLY);

	//send the filename
	sprintf(t.payload, "%s_copy", file);
	t.len = strlen(t.payload)+1;
	t.checksum = get_checksum(t);
	send_message(&t);

	//checks if ACK is received
	check(&t);

	int size = lseek(fp, 0, SEEK_END);

	lseek(fp, 0, SEEK_SET);

	//send file size
	sprintf(t.payload, "%d", size);
	t.len = strlen(t.payload)+1;
	t.checksum = get_checksum(t);
	send_message(&t);

	//checks if ACK is received
	check(&t);

	int sent_size = 0;
  	int fp2;

  	int window_size = (count * 100) / (8 * size);

  	for(int i= 0; i < window_size; i++){

  		fp2 = read(fp, t.payload, MSGSIZE);
  		if(fp2 < 0){
			printf("Reading error\n");
			return;
		}

		sent_size += fp2;

		t.len = fp2;
		t.checksum = get_checksum(t);
		send_message(&t);

	}

	while(sent_size < size){

		fp2 = read(fp, t.payload, MSGSIZE);

		if(fp2 < 0){

			printf("Reading error\n");
			return;
		}

		t.len = fp2;
		t.checksum = get_checksum(t);
		send_message(&t);

    	check(&t);

    	sent_size += fp2;

  	}

	for(int i= 0; i < window_size; i++){
		check(&t);
	}
  
  close(fp);
	
}

int main(int argc, char *argv[])
{
	msg t;
	int i, res;
	
	printf("[SENDER] Starting.\n");	
	init(HOST, PORT);

	/* printf("[SENDER]: BDP=%d\n", atoi(argv[1])); */
	
	for (i = 0; i < COUNT; i++) {
		/* cleanup msg */
		memset(&t, 0, sizeof(msg));
		
		/* gonna send an empty msg */
		t.len = MSGSIZE;
		
		/* send msg */
		res = send_message(&t);
		if (res < 0) {
			perror("[SENDER] Send error. Exiting.\n");
			return -1;
		}
		
		/* wait for ACK */
		res = recv_message(&t);
		if (res < 0) {
			perror("[SENDER] Receive error. Exiting.\n");
			return -1;
		}
	}

	send_file("file.in", t, atoi(argv[1]));

	printf("[SENDER] Job done, all sent.\n");
		
	return 0;
}
