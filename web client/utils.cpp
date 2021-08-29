#include "utils.h"

void compute_message(char *message, char const*line){
    strcat(message, line);
    strcat(message, NEWLINE);
}

int open_connection(int ip_type, int socket_type, int flag) {
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(PORT);
    inet_aton(HOST, &serv_addr.sin_addr);
    // open socket
    int sockfd = socket(ip_type, socket_type, flag);
    DIE (sockfd < 0, "ERROR opening socket.\n");
    // connect the socket
    DIE (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0,
                    "Connection error.\n");
    return sockfd;
}

void close_connection(int sockfd){
    // close socket
    close(sockfd);
}

void send_to_server(int sockfd, char *message){
    int bytes, sent = 0;
    int total = strlen(message);
    do {
        bytes = write(sockfd, message + sent, total - sent);
        DIE (bytes < 0, "ERROR writing message to socket.\n");
        if (bytes == 0)
            break;
        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd){
    char *response = (char*)calloc(BUFLEN, sizeof(char));
    int total = BUFLEN, received = 0;
    do {
        int bytes = read(sockfd, response + received, total - received);
        DIE (bytes < 0, "ERROR reading response from socket.\n");
        if (bytes == 0){
            break;
        }
        received += bytes;
    } while (received < total);
    DIE (received == total,
         "ERROR storing complete response from socket.\n");
    return response;
}

char *compute_get_request(char *url, char *cookie, char* token) {
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    DIE (message == nullptr, "Allocation error.\n");
    char *line = (char *) calloc(LINELEN, sizeof(char));
    DIE (line == nullptr, "Allocation error.\n");
    sprintf(line, "GET %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", HOST);
    compute_message(message, line);
    // add cookies if there are any
    if (cookie != nullptr) {
        sprintf(line, "Cookie: %s", cookie);
        compute_message(message, line);
    }
    // add token if there is one
    if(token != nullptr){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    compute_message(message, "");
    // free memory
    free(line);
    return message;
}

char *compute_post_request(char *url, char *body_data, char* token){
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    DIE (message == nullptr, "Allocation error.\n");
    char *line = (char *)calloc(LINELEN, sizeof(char));
    DIE (line == nullptr, "Allocation error.\n");

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", HOST);
    compute_message(message, line);
    sprintf(line, "Content-Type: application/json");
    compute_message(message, line);
    size_t size = strlen(body_data);
    sprintf(line, "Content-Length: %ld", size);
    compute_message(message, line);
    // add token if there is one
    if(token != nullptr){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    compute_message(message, "");
    memset(line, 0, LINELEN);
    compute_message(message, body_data);
    // free memory
    free(line);
    return message;
}

char *compute_delete_request(char *url, char *token) {
    char *message = (char*)calloc(BUFLEN, sizeof(char));
    DIE (message == nullptr, "Allocation error.\n");
    char *line = (char*)calloc(LINELEN, sizeof(char));
    DIE (line == nullptr, "Allocation error.\n");
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);
    sprintf(line, "Host: %s", HOST);
    compute_message(message, line);
    // add token if there is one
    if(token != nullptr){
        sprintf(line, "Authorization: Bearer %s", token);
        compute_message(message, line);
    }
    compute_message(message, line);
    strcpy(line, "");
    compute_message(message, line);
    // free memory
    free(line);
    return message;
}
