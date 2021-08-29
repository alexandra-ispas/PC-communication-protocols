#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

int main(int argc, char *argv[])
{
    char *message;
    char *response;
    int sockfd;

    // GET dummy from main server
    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.118.48.238", "/api/v1/dummy", NULL, NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);

    // POST dummy and print response from main server
    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

    message = compute_post_request("34.118.48.238", "/api/v1/dummy", "application/x-www-form-urlencoded", NULL, 0, NULL, 0);
    puts(message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);

    // Login into main server
    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

    char **form_data = calloc(2, sizeof(char *));
    for (int i = 0; i < 2; i++) {
        form_data[i] = calloc(LINELEN, sizeof(char));
    }
    strcpy(form_data[0], "username=student");
    strcpy(form_data[1], "password=student");

    message = compute_post_request("34.118.48.238", "/api/v1/auth/login", "application/x-www-form-urlencoded", form_data, 2, NULL, 0);
    puts(message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);
    // GET weather key from main server
    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);

    char **cookies = calloc(1, sizeof(char *));
    cookies[0] = calloc(LINELEN, sizeof(char));
    strcpy(cookies[0], "connect.sid=s%3A0dfyyGJXY0WWqAm2vURQzeRZdq74tDLC.oRAlGPAvvJ221TlEMQc7rzPexwHR6g%2FVbjWncrFTbCY");

    message = compute_get_request("34.118.48.238", "/api/v1/weather/key", NULL, cookies, 1);
    puts(message);

    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);
    // GET weather data from OpenWeather API
    sockfd = open_connection("37.139.20.5", 80, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("37.139.20.5", "/data/2.5/weather",
     "lat=44.439663&lon=26.096306&appid=b912dd495585fbf756dc6d8f415a7649", NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);

    // Logout from main server
    sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
    message = compute_get_request("34.118.48.238", "/api/v1/auth/logout", NULL, NULL, 0);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    puts(response);
    close(sockfd);

    // free the allocated data at the end!
    free(cookies[0]);
    free(cookies);

    for (int i = 0; i < 2; i++) {
        free(form_data[i]);
    }
    free(form_data);
    return 0;
}
