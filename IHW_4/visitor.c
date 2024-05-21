#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define NUM_PICTURES 5
#define MAX_TIME 5

struct ThreadArgs {
    int visitor_id;
    const char *ip_address;
    int port;
};

void random_sleep() {
    sleep(rand() % MAX_TIME + 1);
}

void *visit_gallery(void *args) {
    struct ThreadArgs *thread_args = (struct ThreadArgs *)args;
    int visitor_id = thread_args->visitor_id;
    const char *ip_address = thread_args->ip_address;
    int port = thread_args->port;

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[1024];
    char command[16];

    srand(time(NULL) ^ visitor_id);

    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        pthread_exit(NULL);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_address, &server_addr.sin_addr);

    // Send visitor ID
    sendto(client_socket, &visitor_id, sizeof(visitor_id), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Request to enter the gallery
    strcpy(command, "ENTER");
    sendto(client_socket, command, strlen(command), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    socklen_t server_addr_len = sizeof(server_addr);
    recvfrom(client_socket, buffer, 1024, 0, (struct sockaddr *)&server_addr, &server_addr_len);
    if (strncmp(buffer, "WAIT", 4) == 0) {
        printf("Visitor %d waiting to enter the gallery\n", visitor_id);
        close(client_socket);
        pthread_exit(NULL);
    }

    printf("Visitor %d entered the gallery\n", visitor_id);

    // Array to track which pictures have been viewed
    int viewed_pictures[NUM_PICTURES] = {0};
    int viewed_count = 0;

    // Visit each picture in random order
    while (viewed_count < NUM_PICTURES) {
        int picture_index = rand() % NUM_PICTURES;
        if (viewed_pictures[picture_index] == 1) {
            continue; // Skip if already viewed
        }

        sprintf(command, "MOVE %d", picture_index);
        sendto(client_socket, command, strlen(command), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        recvfrom(client_socket, buffer, 1024, 0, (struct sockaddr *)&server_addr, &server_addr_len);
        if (strncmp(buffer, "WAIT", 4) == 0) {
            printf("Visitor %d waiting to see picture %d\n", visitor_id, picture_index);
            random_sleep(); // Wait before retrying
            continue;
        }

        printf("Visitor %d viewing picture %d\n", visitor_id, picture_index);
        viewed_pictures[picture_index] = 1;
        viewed_count++;
        random_sleep(); // Simulate viewing time

        // Notify server that visitor is leaving the picture
        sprintf(command, "LEAVE %d", picture_index);
        sendto(client_socket, command, strlen(command), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    }

    // Notify server that visitor is leaving the gallery
    strcpy(command, "LEAVE -1");
    sendto(client_socket, command, strlen(command), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Visitor %d left the gallery\n", visitor_id);

    close(client_socket);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS> <PORT> <NUM_VISITORS>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);
    int num_visitors = atoi(argv[3]);

    pthread_t threads[num_visitors];
    struct ThreadArgs thread_args[num_visitors];

    // Create threads for each visitor
    for (int i = 0; i < num_visitors; i++) {
        thread_args[i].visitor_id = i + 1;
        thread_args[i].ip_address = ip_address;
        thread_args[i].port = port;

        if (pthread_create(&threads[i], NULL, visit_gallery, (void *)&thread_args[i]) != 0) {
            perror("pthread_create failed");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < num_visitors; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
