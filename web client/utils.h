#ifndef TEMA3_UTILS_H
#define TEMA3_UTILS_H

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <string>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::ordered_json;

/*
 * Macro from lab
 */
#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define HOST "34.118.48.238"
#define PORT 8080
#define BUFLEN 4096
#define LINELEN 300
#define NEWLINE "\r\n"
#define EXIT "exit"
#define REGISTER "register"
#define LOGIN "login"
#define ENTER "enter_library"
#define GETS "get_books"
#define GET "get_book"
#define ADD "add_book"
#define DELETE "delete_book"
#define LOGOUT "logout"

/*
 * function from lab 10
 */
void compute_message(char *message, const char *line);

/*
 * function from lab 10
 * creates a connection by opening a socket
 */
int open_connection(int ip_type, int socket_type, int flag);

/*
 * function from lab 10
 * closes a socket
 */
void close_connection(int sockfd);

/*
 * function from lab10
 * sends a message to the server
 * */
void send_to_server(int sockfd, char *message);

/*
 * function from lab10
 * receives a message from the server
 */
char *receive_from_server(int sockfd);

/*
 * function from lab 10
 * the number of cookies can be maximum 1
 * and now we can add an authentication token
 */
char *compute_get_request(char *url, char *cookie, char* token);


char *compute_post_request(char *url, char *body_data, char* token);

/*
 * the function sends a 'delete' request
 * if the client has the token
 */
char *compute_delete_request(char *url,char *token) ;

#endif //TEMA3_UTILS_H
