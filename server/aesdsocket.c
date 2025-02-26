#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h> 
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT "9000"
#define BUFFER_SIZE 1024
#define FILE_PATH "/var/tmp/aesdsocketdata"

static volatile int running = 1;

void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        syslog(LOG_INFO, "Caught signal, exiting");
        running = 0;
    }
}

ssize_t handle_client(int client_sock, FILE *file_fd) {

    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_received;
    ssize_t total_bytes_received = 0;

    while (running) {

        if((bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0))== -1){
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                usleep(1000000); // No data backoff for 1msec
                continue;
            }
            else{
                syslog(LOG_ERR,"Failed to recieve data from the file with errono %s",strerror(errno));
                return total_bytes_received;
            }
        }
        if (fwrite(buffer, 1, bytes_received, file_fd) < 0) {
            syslog(LOG_ERR, "Failed to write to file: %s", strerror(errno));
            break;
        }

        total_bytes_received += bytes_received;

        if (strchr(buffer, '\n') != NULL) {
            break;
        }

    }

    return total_bytes_received;
}


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family ==  AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
    int server_sock, client_sock, satrt_as_daemon = 0;
    int optname = 1;

    struct addrinfo hints ,*serverinfo, *temp;
    struct sockaddr_storage  client;
    struct sigaction new_action;

    if (argc >= 2 && strcmp(argv[1], "-d") == 0) {

        satrt_as_daemon = 1;

    }

    openlog("aesdsocket", LOG_PID | LOG_CONS, LOG_USER);

    memset(&new_action, 0 , sizeof(struct sigaction));

    new_action.sa_handler = handle_signal;

    if (sigaction(SIGTERM, &new_action, 0) != 0){
        syslog(LOG_ERR, "Error %s registering for SIGTERM ", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &new_action, 0) != 0){
        syslog(LOG_ERR, "Error %s registering for SIGINT ", strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;  //IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL,PORT,&hints,&serverinfo) < 0) {
        syslog(LOG_ERR, "Failed to get address info %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    for( temp = serverinfo ; temp !=NULL ; temp = temp->ai_next){
    if ((server_sock = socket(temp->ai_family, temp->ai_socktype, temp->ai_protocol)) == -1){
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(server_sock);
        continue;
    }
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optname, sizeof(int)) == -1) {
            syslog(LOG_ERR, "Failed to set socket options with error %s", strerror(errno));
            perror("setsockopt");
            return -1;
    }
    if (bind(server_sock, temp->ai_addr, temp->ai_addrlen) < 0) {
        syslog(LOG_ERR, "Failed to bind socket: %s", strerror(errno));
        close(server_sock);
        continue;
    }
    break;
    }

    if (satrt_as_daemon) {
        pid_t pid = fork();
        if (pid < 0) {
            syslog(LOG_ERR, "Failed to fork process: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS); // Parent process exits
        }
    }
    if (listen(server_sock, 5) < 0) {
        syslog(LOG_ERR, "Failed to listen on socket: %s", strerror(errno));
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    syslog(LOG_INFO, "Server started on port %s", PORT);
    freeaddrinfo(serverinfo);

    while(running) {

        socklen_t sockSize = sizeof(client);
        char client_addr[INET6_ADDRSTRLEN];
        char buffer [BUFFER_SIZE];
        FILE *file_fd;
        ssize_t total;

        client_sock = accept(server_sock, (struct sockaddr *)&client, &sockSize);
        if (client_sock < 0) {
            syslog(LOG_ERR, "Failed to accept connection: %s", strerror(errno));
            continue;
        }
        inet_ntop(client.ss_family, get_in_addr((struct sockaddr *)&client), client_addr, sizeof client);
        syslog(LOG_INFO, "Accepted connection from %s", client_addr);

        file_fd = fopen(FILE_PATH, "a+");

        if (file_fd == NULL) {
        syslog(LOG_ERR, "Failed to open file: %s", strerror(errno));
        close(client_sock);
        continue;
        }

        total = handle_client(client_sock, file_fd);

        if (total == 0) {
        syslog(LOG_ERR, "Failed to write to th file: %s", strerror(errno));
        close(client_sock);
        fclose(file_fd);
        continue;
        }

        size_t bytes_read;
        rewind(file_fd);
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), file_fd)) > 0) {
            send(client_sock, buffer, bytes_read, 0);
        }
        rewind(file_fd);
        syslog(LOG_INFO, "Closed connection from %s", client_addr);
        fclose(file_fd);
        close(client_sock);
    }

    close(server_sock);
    remove(FILE_PATH);
    closelog();

    return 0;
}