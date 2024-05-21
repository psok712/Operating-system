#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int broadcast_port = atoi(argv[1]);
    int sockfd;
    struct sockaddr_in listen_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len;

    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для приема широковещательных сообщений
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(broadcast_port);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Привязка сокета к адресу
    if (bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for broadcast messages on port %d...\n", broadcast_port);

    while (1) {
        addr_len = sizeof(listen_addr);
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&listen_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
        } else {
            buffer[recv_len] = '\0';
            printf("Received broadcast message: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}

