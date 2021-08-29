#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "link_emulator/lib.h"

#define HOST "127.0.0.1"
#define PORT 10001

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

}

void receive_file(msg r){

  check(&r);

  int fp = open(r.payload, O_WRONLY | O_CREAT | O_TRUNC, 0644);

  sendACK(r);

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

int main(int argc,char** argv){

  msg r;
  init(HOST,PORT);

  check(&r);

  sendACK(r);

  receive_file(r);

  return 0;
}

