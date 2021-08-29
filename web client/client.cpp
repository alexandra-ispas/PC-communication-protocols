#include "actions.h"

int main(){
    char command[100];
    // payload for storing the json message
    // sent to the server
    char *payload = (char*)calloc(BUFLEN, sizeof(char));
    DIE (payload == nullptr, "Allocation error.\n");
    // the cookie which proves the user is logged in
    char *cookie = (char*)calloc(BUFLEN, sizeof(char));
    DIE (cookie == nullptr, "Allocation error.\n");
    // token which proves the user has entered the library
    char *token = (char*)calloc(BUFLEN, sizeof(char));
    DIE (token == nullptr, "Allocation error.\n");
    // empty the char arrays
    strcpy(cookie, "");
    strcpy(token, "");

    while(true){
        // read the command from stdin
        cin >> command;
        if(strcmp(command, EXIT) == 0){
            // if command 'exit' is received,
            // stop the programme, but free memory first
            free(payload);
            free(cookie);
            free(token);
            return 0;
        } else if(strcmp(command, REGISTER) == 0){
            // the client wants to register
            register_client(payload);
        } else if(strcmp(command, LOGIN) == 0){
            // the client wants to log in
            login_client(&cookie, payload);
        } else if(strcmp(command, ENTER) == 0) {
            // the client wants to enter the library
            enter_library(&token, cookie);
        } else if(strcmp(command, GETS) == 0){
            // the client wants to receive information
            // about all the books in the library
            get_books(token);
        } else if(strcmp(command, GET) == 0){
            // the client wants to receive information
            // about a specific book in the library
            get_book(token);
        } else if(strcmp(command, ADD) == 0){
            // the client wants to add a book to the library
            add_book(payload, token);
        } else if(strcmp(command, DELETE) == 0){
            // the client wants to delete a book from the library
            delete_book(token);
        } else if(strcmp(command, LOGOUT) == 0){
            // the client wants to log out
            // empty the cookie and token char array, because
            // these details should not be known anymore
            logout_client(cookie);
            strcpy(cookie, "");
            strcpy(token, "");
        }
    }
}
