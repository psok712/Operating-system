#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <MULTICAST_IP> <PORT>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *multicast_ip = argv[1];
    int multicast_port = atoi(argv[2]);
    int sockfd;
    struct sockaddr_in listen_addr;
    char buffer[BUFFER_SIZE];
    struct ip_mreq mreq;

    // Создание UDP сокета
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для приема многоадресных сообщений
    memset(&listen_addr, 0, sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(multicast_port);
    listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Привязка сокета к адресу
    if (bind(sockfd, (struct sockaddr*)&listen_addr, sizeof(listen_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Присоединение к многоадресной группе
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Listening for multicast messages on %s:%d...\n", multicast_ip, multicast_port);

    while (1) {
        socklen_t addr_len = sizeof(listen_addr);
        int recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr*)&listen_addr, &addr_len);
        if (recv_len < 0) {
            perror("recvfrom");
        } else {
            buffer[recv_len] = '\0';
            printf("Received multicast message: %s\n", buffer);
        }
    }

    close(sockfd);
    return 0;
}

