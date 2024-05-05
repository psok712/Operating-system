#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#define MAX_PENDING 5
#define BUFFER_SIZE 32

int main(int argc, char *argv[]) {
    int serversock, clientsock1, clientsock2;
    struct sockaddr_in echoServAddr, echoClient;
    char buffer[BUFFER_SIZE];
    unsigned int recvMsgSize;
    int totalBytesRcvd;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> <Server Port>\n", argv[0]);
        exit(1);
    }

    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Ошибка создания сокета");
        exit(1);
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    echoServAddr.sin_port = htons(atoi(argv[2]));

    if (bind(serversock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("Ошибка привязки сокета сервера");
        exit(1);
    } else {
        fprintf(stdout, "Сокет сервера привязан\n");
    }

    if (listen(serversock, MAX_PENDING) < 0) {
        perror("Ошибка при прослушивании сокета сервера");
        exit(1);
    }
    printf("IP адрес сервера = %s. Ожидание...\n", inet_ntoa(echoServAddr.sin_addr));

    while (true) {
        recvMsgSize = sizeof(echoClient);

        if ((clientsock1 = accept(serversock, (struct sockaddr *) &echoClient, &recvMsgSize)) < 0) {
            perror("Ошибка при принятии соединения с клиентом");
            exit(1);
        } else {
            fprintf(stdout, "Клиент подключен: %s\n", inet_ntoa(echoClient.sin_addr));
        }

        if ((clientsock2 = accept(serversock, (struct sockaddr *) &echoClient, &recvMsgSize)) < 0) {
            perror("Ошибка при принятии соединения с клиентом");
            exit(1);
        } else {
            fprintf(stdout, "Второй клиент подключен: %s\n", inet_ntoa(echoClient.sin_addr));
        }

        if ((totalBytesRcvd = recv(clientsock1, buffer, BUFFER_SIZE, 0)) < 0) {
            perror("Ошибка при получении данных от клиента");
            exit(1);
        }
        buffer[totalBytesRcvd] = '\0';

        if (strcmp(buffer, "конец\n") == 0) {
            break;
        }
        bool flag = false;

        while (totalBytesRcvd > 0) {
            if (send(clientsock2, buffer, totalBytesRcvd, 0) != totalBytesRcvd) {
                perror("Ошибка при отправке данных клиенту");
                exit(1);
            } else {
                fprintf(stdout, "Обработка сервером\n");
            }
            if (strcmp(buffer, "конец\n") == 0) {
                flag = true;
                break;
            }
            if ((totalBytesRcvd = recv(clientsock1, buffer, BUFFER_SIZE, 0)) < 0) {
                perror("Ошибка при получении дополнительных данных от клиента");
                exit(1);
            }
            buffer[totalBytesRcvd] = '\0';

        }
        if (flag) {
            break;
        }

    }
    close(clientsock1);
    close(clientsock2);
    return 0;
}