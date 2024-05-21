#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <BROADCAST_IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *broadcast_ip = argv[1];
    int broadcast_port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in broadcast_addr;
    char buffer[BUFFER_SIZE];

    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Разрешение на широковещательную рассылку
    int broadcast_enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_enable, sizeof(broadcast_enable)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для широковещательной рассылки
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(broadcast_port);
    broadcast_addr.sin_addr.s_addr = inet_addr(broadcast_ip);

    while (1) {
        printf("Enter message to broadcast: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            // Удаление символа новой строки, если он есть
            buffer[strcspn(buffer, "\n")] = '\0';

            // Отправка сообщения
            if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) < 0) {
                perror("sendto");
            } else {
                printf("Broadcast message sent: %s\n", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}

