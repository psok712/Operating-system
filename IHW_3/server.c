#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_VISITORS 25
#define MAX_VIEWERS_PER_PICTURE 5
#define NUM_PICTURES 5
#define QUEUE_SIZE 10

int current_visitors = 0;
int picture_viewers[NUM_PICTURES] = {0};
pthread_mutex_t lock;

void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    char buffer[1024];
    int visitor_id;

    // Receive visitor ID
    if (recv(client_socket, &visitor_id, sizeof(visitor_id), 0) <= 0) {
        close(client_socket);
        return NULL;
    }
    printf("Visitor %d connected.\n", visitor_id);

    while (1) {
        bzero(buffer, 1024);
        int bytes_received = recv(client_socket, buffer, 1024, 0);
        if (bytes_received <= 0) {
            printf("Visitor %d disconnected.\n", visitor_id);
            break;
        }
        printf("Visitor %d sent command: %s\n", visitor_id, buffer);

        if (strncmp(buffer, "ENTER", 5) == 0) {
            pthread_mutex_lock(&lock);
            if (current_visitors < MAX_VISITORS) {
                current_visitors++;
                send(client_socket, "OK", 2, 0);
                printf("Visitor %d entered the gallery. Current visitors: %d\n", visitor_id, current_visitors);
            } else {
                send(client_socket, "WAIT", 4, 0);
                printf("Visitor %d has to wait. Gallery is full.\n", visitor_id);
                pthread_mutex_unlock(&lock);
                continue; // Continue waiting for commands
            }
            pthread_mutex_unlock(&lock);
        } else if (strncmp(buffer, "MOVE", 4) == 0) {
            int picture_index = atoi(buffer + 5);
            pthread_mutex_lock(&lock);
            if (picture_index >= 0 && picture_index < NUM_PICTURES) {
                if (picture_viewers[picture_index] < MAX_VIEWERS_PER_PICTURE) {
                    picture_viewers[picture_index]++;
                    send(client_socket, "OK", 2, 0);
                    printf("Visitor %d viewing picture %d. Viewers at picture %d: %d\n", visitor_id, picture_index, picture_index, picture_viewers[picture_index]);
                } else {
                    send(client_socket, "WAIT", 4, 0);
                    printf("Visitor %d has to wait. Picture %d is full.\n", visitor_id, picture_index);
                    pthread_mutex_unlock(&lock);
                    continue; // Continue waiting for commands
                }
            } else {
                send(client_socket, "ERROR", 5, 0);
                printf("Visitor %d sent an invalid MOVE command.\n", visitor_id);
            }
            pthread_mutex_unlock(&lock);
        } else if (strncmp(buffer, "LEAVE", 5) == 0) {
            int picture_index = atoi(buffer + 6);
            pthread_mutex_lock(&lock);
            if (picture_index >= 0 && picture_index < NUM_PICTURES) {
                picture_viewers[picture_index]--;
                printf("Visitor %d left picture %d. Viewers at picture %d: %d\n", visitor_id, picture_index, picture_index, picture_viewers[picture_index]);
            }
            current_visitors--;
            pthread_mutex_unlock(&lock);
            printf("Visitor %d left the gallery. Current visitors: %d\n", visitor_id, current_visitors);
            if (current_visitors <= 0) {
                break; // Exit the loop if no more visitors
            }
        } else {
            send(client_socket, "ERROR", 5, 0);
            printf("Visitor %d sent an invalid command.\n", visitor_id);
        }
    }

    close(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int server_socket, client_socket;
    struct sockaddr_in server_addr;
    socklen_t client_len;
    pthread_t tid;

    pthread_mutex_init(&lock, NULL);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, QUEUE_SIZE) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on %s:%d...\n", ip_address, port);

    signal(SIGINT, sigint_handler);

    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Server accept failed");
            close(server_socket);
            pthread_mutex_destroy(&lock);
            exit(EXIT_FAILURE);
        }

        int *client_sock_ptr = malloc(sizeof(int));
        *client_sock_ptr = client_socket;
        pthread_create(&tid, NULL, handle_client, client_sock_ptr);
    }
}
