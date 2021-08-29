#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

void check_checksum(msg r){
	int sum = 0;
	for(int i = 0; i < MSGSIZE; i++){
		for(int j = 0; j < 8; j++){
			sum ^= (1 << j) & r.payload[i];
		}
	}

	if(r.checksum != sum){
		printf("[ERROR] checksum %d %d\n", r.checksum, sum);
	}
}

void sendACK(msg r){

  sprintf(r.payload,"%s", "ACK");
  r.len = strlen(r.payload) + 1;
  send_message(&r);
  printf("[recv] ACK sent\n");
}

void check(msg *r){

  if (recv_message(r)<0){
    perror("Receive message");
    return ;
  }

  printf("[recv] Got msg \n");

  //verify if the checksum is ok
  check_checksum(*r);
}

void receive_file(msg r){

	//receive filename
	check(&r);

	int fp = open(r.payload, O_WRONLY | O_CREAT | O_TRUNC, 0644);

	sendACK(r);

	//receive the size of the file
	check(&r);
	  
	int size = atoi(r.payload);

	sendACK(r);

	int received_size = 0;

	while(received_size < size){

	    check(&r);

	    write(fp, r.payload, r.len);

	    received_size += r.len;

	   	sendACK(r);
	}

  close(fp);

}

int main(void)
{
	msg r;
	int i, res;
	
	printf("[RECEIVER] Starting.\n");
	init(HOST, PORT);
	
	for (i = 0; i < COUNT; i++) {
		/* wait for message */
		res = recv_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Receive error. Exiting.\n");
			return -1;
		}
		
		/* send dummy ACK */
		res = send_message(&r);
		if (res < 0) {
			perror("[RECEIVER] Send ACK error. Exiting.\n");
			return -1;
		}
	}


	receive_file(r);

	printf("[RECEIVER] Finished receiving..\n");
	return 0;
}
