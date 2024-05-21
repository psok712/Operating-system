#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void *receive_info(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t server_addr_len = sizeof(server_addr);

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        int bytes_received = recvfrom(client_socket, buffer, BUFFER_SIZE, 0,
                                      (struct sockaddr *)&server_addr, &server_addr_len);
        if (bytes_received < 0) {
            perror("Error in recvfrom");
            pthread_exit(NULL);
        }
        printf("INFO: %s\n", buffer);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t tid;

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address, &server_addr.sin_addr);

    printf("Connected to server %s:%d\n", ip_address, port);

    pthread_create(&tid, NULL, receive_info, &client_socket);

    // Main loop to send complex application information
    char info_message[BUFFER_SIZE];
    int counter = 0;

    while (1) {
        snprintf(info_message, BUFFER_SIZE, "command: Complex app info %d", counter);
        printf("INFO: %s\n", info_message); // Display the sent message
        sendto(client_socket, info_message, strlen(info_message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr));

        counter++;
        sleep(1); // Adjust this time interval as needed
    }

    pthread_join(tid, NULL);

    close(client_socket);

    return 0;
}
