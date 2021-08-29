#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10000

void check(msg *t){

	if (recv_message(t)<0)
	   perror("Receive error ...");
	else 
	 printf("[send] Got reply\n");
}

void send_file(char* file, msg t){

	int fp = open(file, O_RDONLY);

	sprintf(t.payload, "%s_copy", file);
	t.len = strlen(t.payload)+1;
	send_message(&t);

	check(&t);

	//get file size
	int size = lseek(fp, 0, SEEK_END);

	lseek(fp, 0, SEEK_SET);

	sprintf(t.payload, "%d", size);
	t.len = strlen(t.payload)+1;
	send_message(&t);

	check(&t);

	int sent_size = 0;
  int fp2;

	while(sent_size < size){

		fp2 = read(fp, t.payload, MAX_LEN);


		if(fp2 < 0){
			printf("Reading error\n");
			return;
		}

		t.len = fp2;

		send_message(&t);

    check(&t);

    sent_size += fp2;

  }
  
  close(fp);
	
}

int main(int argc,char** argv){

	init(HOST,PORT);
	msg t;

	//Send dummy message:
	printf("[send] Sending dummy...\n");
	sprintf(t.payload,"%s", "This is a dummy.");
	t.len = strlen(t.payload)+1;
	send_message(&t);
	  
	// Check response:
	check(&t);

	send_file(argv[1], t);

	return 0;
}
