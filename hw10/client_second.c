#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#define BUFFER_SIZE 32

int main(int argc, char *argv[]) {
    int sock;
    struct sockaddr_in echoServAddr;
    char buffer[BUFFER_SIZE];
    int totalBytesRcvd;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "USAGE: client <server_ip> <port>\n");
        exit(1);
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    echoServAddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("Ошибка подключения к серверу");
        exit(1);
    }
    printf("Ожидание данных от сервера...\n");

    while (true) {
        if ((totalBytesRcvd = recv(sock, buffer, BUFFER_SIZE - 1, 0)) <= 0) {
            perror("Не удалось получить данные от сервера");
            exit(1);
        }

        buffer[totalBytesRcvd] = '\0';

        printf("Получено: %s", buffer);

        if (strcmp(buffer, "конец\n") == 0) {
            close(sock);
            break;
        }
    }

    return 0;
}
