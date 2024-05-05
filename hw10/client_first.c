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
    unsigned int recvMsgSize;

    if ((argc < 3) || (argc > 4)) {
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    echoServAddr.sin_port = htons(atoi(argv[2]));

    if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("Ошибка при подключении к серверу");
        exit(1);
    }

    while (true) {
        printf("Введите сообщение: ");
        fgets(buffer, BUFFER_SIZE, stdin);
        recvMsgSize = strlen(buffer);

        if (send(sock, buffer, recvMsgSize, 0) != recvMsgSize) {
            perror("Несоответствие количества отправленных байтов");
            exit(1);
        }

        if (strcmp(buffer, "конец\n") == 0) {
            close(sock);
            break;
        }
    }

    return 0;
}
