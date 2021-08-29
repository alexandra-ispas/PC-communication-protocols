#
#  	Protocoale de comunicatii: 
#  	Laborator 6: UDP
#	mini-server de backup fisiere
# 	Makefile

CFLAGS = -Wall -g

# Portul pe care asculta serverul
PORT = 10302

# Adresa IP a serverului
IP_SERVER = 127.0.0.1

# Fisierul de unde citeste clientul
FILE_CLIENT = client_random
FILE_CLIENT2 = client_random2
FILE_CLIENT3 = client_random3

# Fisierul unde salveaza serverul
FILE_SERVER = server_random
FILE_SERVER2 = server_random2
FILE_SERVER3 = server_random3

all: server client 

# Compileaza server.c
server: server.c

# Compileaza client.c
client: client.c

.PHONY: clean random_file run_server run_client

# Construieste un fisier de 10MB (1024B*10^5)==10MB
random_file:
	dd if=/dev/urandom of=${FILE_CLIENT} bs=1024 count=10
	dd if=/dev/urandom of=${FILE_CLIENT2} bs=1024 count=10
	dd if=/dev/urandom of=${FILE_CLIENT3} bs=1024 count=10

# Ruleaza serverul
run_server:
	./server ${PORT}

# Ruleaza clientul 	
run_client:
	./client ${IP_SERVER} ${PORT} ${FILE_CLIENT} ${FILE_CLIENT2} ${FILE_CLIENT3}

clean:
	rm -f server client server_* client_*
