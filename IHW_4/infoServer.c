#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define MAX_VISITORS 25
#define MAX_VIEWERS_PER_PICTURE 5
#define NUM_PICTURES 5
#define BUFFER_SIZE 1024

int current_visitors = 0;
int picture_viewers[NUM_PICTURES] = {0};
pthread_mutex_t lock;

// Функция для отправки информации на информационный клиент
void send_info(const char *info, struct sockaddr_in *info_client_addr, socklen_t info_client_len, int server_socket) {
    sendto(server_socket, info, strlen(info), 0, (struct sockaddr *)info_client_addr, info_client_len);
}

// Обработчик сигналов для корректного завершения программы
void sigint_handler(int sig) {
    exit(0);
}

void *handle_client(void *arg) {
    int server_socket = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int visitor_id;

    // Получение ID посетителя
    if (recvfrom(server_socket, &visitor_id, sizeof(visitor_id), 0, (struct sockaddr *)&client_addr, &client_len) <= 0) {
        return NULL;
    }

    printf("Visitor %d connected.\n", visitor_id);
    char info[256];
    snprintf(info, sizeof(info), "Visitor %d connected.\n", visitor_id);
    send_info(info, &client_addr, client_len, server_socket);

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        int bytes_received = recvfrom(server_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_len);
        if (bytes_received <= 0) {
            printf("Visitor %d disconnected.\n", visitor_id);
            snprintf(info, sizeof(info), "Visitor %d disconnected.\n", visitor_id);
            send_info(info, &client_addr, client_len, server_socket);
            break;
        }

        printf("Visitor %d sent command: %s\n", visitor_id, buffer);
        snprintf(info, sizeof(info), "Visitor %d sent command: %s\n", visitor_id, buffer);
        send_info(info, &client_addr, client_len, server_socket);

        if (strncmp(buffer, "ENTER", 5) == 0) {
            pthread_mutex_lock(&lock);
            if (current_visitors < MAX_VISITORS) {
                current_visitors++;
                sendto(server_socket, "OK", 2, 0, (struct sockaddr *)&client_addr, client_len);
                snprintf(info, sizeof(info), "Visitor %d entered the gallery. Current visitors: %d\n", visitor_id, current_visitors);
                printf("%s", info);
                send_info(info, &client_addr, client_len, server_socket);
            } else {
                sendto(server_socket, "WAIT", 4, 0, (struct sockaddr *)&client_addr, client_len);
                snprintf(info, sizeof(info), "Visitor %d has to wait. Gallery is full.\n", visitor_id);
                printf("%s", info);
                send_info(info, &client_addr, client_len, server_socket);
                pthread_mutex_unlock(&lock);
                continue;
            }
            pthread_mutex_unlock(&lock);
        } else if (strncmp(buffer, "MOVE", 4) == 0) {
            int picture_index = atoi(buffer + 5);
            pthread_mutex_lock(&lock);
            if (picture_index >= 0 && picture_index < NUM_PICTURES) {
                if (picture_viewers[picture_index] < MAX_VIEWERS_PER_PICTURE) {
                    picture_viewers[picture_index]++;
                    sendto(server_socket, "OK", 2, 0, (struct sockaddr *)&client_addr, client_len);
                    snprintf(info, sizeof(info), "Visitor %d viewing picture %d. Viewers at picture %d: %d\n", visitor_id, picture_index, picture_index, picture_viewers[picture_index]);
                    printf("%s", info);
                    send_info(info, &client_addr, client_len, server_socket);
                } else {
                    sendto(server_socket, "WAIT", 4, 0, (struct sockaddr *)&client_addr, client_len);
                    snprintf(info, sizeof(info), "Visitor %d has to wait. Picture %d is full.\n", visitor_id, picture_index);
                    printf("%s", info);
                    send_info(info, &client_addr, client_len, server_socket);
                    pthread_mutex_unlock(&lock);
                    continue;
                }
            } else {
                sendto(server_socket, "ERROR", 5, 0, (struct sockaddr *)&client_addr, client_len);
                snprintf(info, sizeof(info), "Visitor %d sent an invalid MOVE command.\n", visitor_id);
                printf("%s", info);
                send_info(info, &client_addr, client_len, server_socket);
            }
            pthread_mutex_unlock(&lock);
        } else if (strncmp(buffer, "LEAVE", 5) == 0) {
            int picture_index = atoi(buffer + 6);
            pthread_mutex_lock(&lock);
            if (picture_index >= 0 && picture_index < NUM_PICTURES) {
                picture_viewers[picture_index]--;
                snprintf(info, sizeof(info), "Visitor %d left picture %d. Viewers at picture %d: %d\n", visitor_id, picture_index, picture_index, picture_viewers[picture_index]);
                printf("%s", info);
                send_info(info, &client_addr, client_len, server_socket);
            }
            current_visitors--;
            pthread_mutex_unlock(&lock);
            snprintf(info, sizeof(info), "Visitor %d left the gallery. Current visitors: %d\n", visitor_id, current_visitors);
            printf("%s", info);
            send_info(info, &client_addr, client_len, server_socket);
            if (current_visitors <= 0) {
                break;
            }
        } else {
            sendto(server_socket, "ERROR", 5, 0, (struct sockaddr *)&client_addr, client_len);
            snprintf(info, sizeof(info), "Visitor %d sent an invalid command.\n", visitor_id);
            printf("%s", info);
            send_info(info, &client_addr, client_len, server_socket);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <IP_ADDRESS> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip_address = argv[1];
    int port = atoi(argv[2]);

    int server_socket;
    struct sockaddr_in server_addr;

    pthread_mutex_init(&lock, NULL);

    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    bzero(&server_addr, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server started on %s:%d...\n", ip_address, port);

    signal(SIGINT, sigint_handler);

    pthread_t tid;
    while (1) {
        int *server_sock_ptr = malloc(sizeof(int));
        *server_sock_ptr = server_socket;
        pthread_create(&tid, NULL, handle_client, server_sock_ptr);
        pthread_detach(tid);  // Детачим поток, чтобы не занимать системные ресурсы
    }

    pthread_mutex_destroy(&lock);
    close(server_socket);
    return 0;
}
