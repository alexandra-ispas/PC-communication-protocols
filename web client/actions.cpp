#include "actions.h"

void register_client(char *payload){
    json jsonAux;
    char read[100], url[100];
    // get input data
    cout << "username=";
    cin >> read;
    jsonAux["username"] = read;
    cout << "password=";
    cin >> read;
    jsonAux["password"] = read;
    // the json object's content is put inside a char*
    // to be sent to the server
    strcpy(payload, jsonAux.dump(4).c_str());
    // add the url
    strcpy(url, "/api/v1/tema/auth/register");
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    // create message
    char *message = compute_post_request(url, payload, nullptr);
    // send message to the server
    send_to_server(sockfd, message);
    // get response fom server
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    // check the received message
    if(strstr(response, "Created") != nullptr){
        cout << "[SUCCESS]: client " << jsonAux.at("username") << " registered.\n";
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}

void login_client(char **cookie, char *payload){
    json jsonAux;
    char read[100], url[100];
    // get input data
    cout << "username=";
    cin >> read;
    jsonAux["username"] = read;
    cout << "password=";
    cin >> read;
    jsonAux["password"] = read;
    // create payload from json
    strcpy(payload, jsonAux.dump(4).c_str());
    // add url
    strcpy(url, "/api/v1/tema/auth/login");
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    char *message = compute_post_request(url, payload,  nullptr);
    // send message to server
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    if(strstr(response, "OK") != nullptr){
        // if the operation was successful
        cout << "[SUCCESS]: client " << jsonAux.at("username") <<" logged in." << endl;
    } else {
        // if no, print the server error message
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[FAILED]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
        // free memory in case the operation was not successful
        // and exit the function
        free(response);
        free(message);
        return;
    }
    // get the cookie which proves that the user has been authenticated
    char* aux = strstr(response, "Set-Cookie: ");
    char *p = strtok(aux, " ");
    p = strtok(nullptr, " ");
    // the last character in the string is ';'
    // and it is not part of the cookie
    strncpy(*cookie, p, strlen(p)-1);
    // free memory
    free(response);
    free(message);
}

void enter_library(char **token, char *cookie){
    // guard
    if(strcmp(cookie, "") == 0){
        cout << "Login you must!" << endl;
        return;
    }
    char url[100];
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    // add url
    strcpy(url, "/api/v1/tema/library/access");
    char *message = compute_get_request(url, cookie, nullptr);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    // check if the operation was successful
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: entered library.\n";
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
        // free memory
        free(response);
        free(message);
        return;
    }
    // parse response in order to get the authentication token
    char *aux = strstr(response, "{");
    json j = json ::parse(aux);
    std::string str = j.value("token", "");
    strncpy(*token, str.c_str(), str.size() + 1);
    // free memory
    free(response);
    free(message);
}

void get_books(char *token){
    // guard
    if(strcmp(token, "") == 0){
        cout << "Enter the library you must!" << endl;
        return;
    }
    char url[100];
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    // add url
    strcpy(url, "/api/v1/tema/library/books");
    // build message
    char *message = compute_get_request(url, nullptr, token);
    // send message to server
    send_to_server(sockfd, message);
    // receive message from the server
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    // check response
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: information about the books:" << endl;
        // print information about the books
        json j = json::parse(strstr(response, "["));
        if(j.empty()){
            cout << "There is no book in the library.\n";
        } else {
            for (auto & it : j){
                cout << "Book: id=" << it.at("id") << ", " << "title=" <<
                     it.at("title") << endl;
            }
        }
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}

void get_book(char *token){
    //guard
    if(strcmp(token, "") == 0){
        cout << "Enter the library you must!" << endl;
        return;
    }
    char id[10], url[100];
    // get id of the book
    cout << "id=";
    cin >> id;
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    // add url
    sprintf(url, "/api/v1/tema/library/books/%s", id);
    // create message
    char *message = compute_get_request(url, nullptr, token);
    send_to_server(sockfd, message);
    // receive response
    char *response = receive_from_server(sockfd);

    // close socket
    close_connection(sockfd);
    // check if the operation was successfull
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: information about the book:\n";
        // print information about the book
        json j = json::parse(strstr(response, "["));
        for (auto & it : j) {
            cout << "title = " <<it.at("title") << endl;
            cout << "author = " << it.at("author") << endl;
            cout << "publisher = " << it.at("publisher") << endl;
            cout << "genre = " << it.at("genre") << endl;
            cout << "page_count = " << it.at("page_count") << endl;
        }
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}

void add_book(char *payload, char *token){
    //guard
    if(strcmp(token, "") == 0){
        cout << "Enter the library you must!" << endl;
        return;
    }
    char read[100], url[100];
    json aux;
    // get information about the new book
    cout << "title=";
    cin >> read;
    aux["title"] = read;    // add title
    cout << "author=";
    cin >> read;
    aux["author"] = read;   // add author
    cout << "genre=";
    cin >> read;
    aux["genre"] = read;    // add genre
    cout << "publisher=";
    cin >> read;
    aux["publisher"] = read;   // add publisher
    cout << "page_count=";
    cin >> read;
    if(atoi(read) == 0){        //check if the number of pages is valid
        cout << "page_count field is not a valid number!\n"
                "Please introduce the correct value:\n"
                "page_count=";
        fscanf(stdin, "%s", read);
    }
    aux["page_count"] = read;   // add page count
    // create the char* payload from json object
    strcpy(payload, aux.dump(4).c_str());
    // add url
    strcpy(url, "/api/v1/tema/library/books");
    // create message
    char *message = compute_post_request(url, payload, token);
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    // send message to server
    send_to_server(sockfd, message);
    // receive message from server
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    // check if the operation was successful
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: book " << aux.at("title") <<" added." << endl;
    } else {
        // if no, print the server error message
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}

void logout_client(char *cookie){
    //guard
    if(strcmp(cookie, "") == 0){
        cout << "Login you must!" << endl;
        return;
    }
    char url[100];
    // add url
    strcpy(url, "/api/v1/tema/auth/logout");
    // create message
    char *message = compute_get_request(url, cookie, nullptr);
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    //close socket
    close_connection(sockfd);
    // check sever response
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: client logged out.\n";
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}

void delete_book(char *token){
    //guard
    if(strcmp(token, "") == 0){
        cout << "Enter the library you must!" << endl;
        return;
    }
    char id[10], url[100];
    // get information about the boook
    cout << "id=";
    cin >> id;
    // add url
    sprintf(url, "/api/v1/tema/library/books/%s", id);
    // create message
    char *message = compute_delete_request(url, token);
    // open socket
    int sockfd = open_connection(AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    char *response = receive_from_server(sockfd);
    // close socket
    close_connection(sockfd);
    // check sever response
    if(strstr(response, "OK") != nullptr){
        cout << "[SUCCESS]: book with id=" << id <<" deleted.\n";
    } else {
        if(strstr(response,
                  "Too many requests") == nullptr) {
            json j = json::parse(strstr(response, "{"));
            cout << "[ERROR]: " << j.at("error") << endl;
        } else {
            cout << "Too many requests, please try again later." << endl;
        }
    }
    // free memory
    free(message);
    free(response);
}
