#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <MULTICAST_IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *multicast_ip = argv[1];
    int multicast_port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in multicast_addr;
    char buffer[BUFFER_SIZE];

    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для многоадресной рассылки
    memset(&multicast_addr, 0, sizeof(multicast_addr));
    multicast_addr.sin_family = AF_INET;
    multicast_addr.sin_port = htons(multicast_port);
    multicast_addr.sin_addr.s_addr = inet_addr(multicast_ip);

    while (1) {
        printf("Enter message to multicast: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) != NULL) {
            // Удаление символа новой строки, если он есть
            buffer[strcspn(buffer, "\n")] = '\0';

            // Отправка сообщения
            if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr*)&multicast_addr, sizeof(multicast_addr)) < 0) {
                perror("sendto");
            } else {
                printf("Multicast message sent: %s\n", buffer);
            }
        }
    }

    close(sockfd);
    return 0;
}

