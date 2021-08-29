#ifndef TEMA3_ACTIONS_H
#define TEMA3_ACTIONS_H

#include "utils.h"

/*
 * registers a new client by creating the suitable json object
 * and sending the message to the specified url.
 */
void register_client(char *payload);

/*
 * a client is logged in with the data from the stdin.
 * A cookie is received in the response from the sever
 * and it is extracted by parsing the payload. It proves
 * that the user has been authenticated.
 */
void login_client(char **cookie, char *payload);

/*
 * the client must prove that he is logged in before
 * performing this action. If the operation succeeds,
 * the client receives an authentication token
 * which is parsed from a json object
 */
void enter_library(char **token, char *cookie);

/*
 * the client must first prove that he has the authentication
 * token. The message is sent with the authentication header
 * completed and the server's response contains information
 * about the books which is accessed by parsing a json array
 */
void get_books(char *token);

/*
 * if the client has already entered the library,
 * he can get information about a specific book.
 * The detailed are printed after having parsed a
 * json array.
 */
void get_book(char *token);
/*
 * a new book can be added by the client only if he has access
 * to the library. The payload is filled with data about the book
 * placed in a json object.
 */
void add_book(char *payload, char *token);

/*
 * the current logged in client can be logged out from
 * the server.
 */
void logout_client(char *cookie);

/*
 * the client can delete a book if he has already
 * entered the library and specifies the book's id
 */
void delete_book(char *token);

#endif //TEMA3_ACTIONS_H