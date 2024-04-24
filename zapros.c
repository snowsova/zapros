#include <sys/socket.h>

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define MAX_BUFFER_SIZE 2048
#define DEFAULT_PORT 80
#define USER_AGENT "test/1.0"


void
parse_url(const char *url, char *protocol, char *host, int *port, char *path) {
        const char *protocol_end = strstr(url, "://");
        if (protocol_end != NULL) {
                int protocol_length = protocol_end - url;
                strncpy(protocol, url, protocol_length);
                protocol[protocol_length] = '\0';
                url = protocol_end + 3;
        } else {
                protocol[0] = '\0';
        }

        const char *port_start = strchr(url, ':');
        const char *path_start = strchr(url, '/');
        if (port_start != NULL && (path_start == NULL || port_start < path_start)) {
                int host_length = port_start - url;
                strncpy(host, url, host_length);
                host[host_length] = '\0';
                *port = atoi(port_start + 1);
        } else {
                if (path_start != NULL) {
                        int host_length = path_start - url;
                        strncpy(host, url, host_length);
                        host[host_length] = '\0';
                } else {
                        strcpy(host, url);
                }
                *port = DEFAULT_PORT;
        }

        if (path_start != NULL) {
                strcpy(path, path_start);
        } else {
                path[0] = '/';
                path[1] = '\0';
        }
}



int
main(int argc, char *argv[])
{
        if (argc < 2) {
                fprintf(stderr, "usage: %s <url>\n", argv[0]);
                return EXIT_FAILURE;
        }

        char request[MAX_BUFFER_SIZE];
        struct sockaddr_in server_addr;
        struct hostent *server;

        char protocol[10], hostname[100], path[100];
        int port;

        parse_url(argv[1], protocol, hostname, &port, path);

        if (strcmp(protocol, "http") != 0 && protocol[0] != '\0') {
                fprintf(stderr, "Only http:// protocol is supported\n");
                return EXIT_FAILURE;
        }


        server = gethostbyname(hostname);
        if (server == NULL) {
                fprintf(stderr, "Could not reslove hostname: %s\n", hostname);
                return EXIT_FAILURE;
        }

        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
                perror("Could not create socket");
                return EXIT_FAILURE;
        }

        server_addr.sin_family = AF_INET;
        bcopy(server->h_addr, &server_addr.sin_addr.s_addr, server->h_length);
        server_addr.sin_port = htons(port);

        if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
                perror("Could not connect to server");
                return EXIT_FAILURE;
        }

        snprintf(request, MAX_BUFFER_SIZE, "GET %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n", path, hostname, USER_AGENT);


        if (send(sockfd, request, strlen(request), 0) < 0) {
                perror("Could not send request");
                return EXIT_FAILURE;
        }

        char response[MAX_BUFFER_SIZE];
        ssize_t bytes_received;
        int header_end = 0;
        while ((bytes_received = recv(sockfd, response, MAX_BUFFER_SIZE - 1, 0)) > 0) {
                response[bytes_received] = '\0';
                char *body = strstr(response, "\r\n\r\n") ? : strstr(response, "\n\n");
                if (body != NULL) {
                        int char_to_skip = strstr(body, "\r\n\r\n") ? 4 : 2;
                        fprintf(stdout, "%s", body + char_to_skip);
                        header_end = 1;
                }
                else if (header_end) {
                        fprintf(stdout, "%s", response);
                }
        }

        if (bytes_received < 0) {
                perror("Could not receive response");
                return EXIT_FAILURE;
        }

        printf("\n");

        close(sockfd);
        return EXIT_SUCCESS;
}

