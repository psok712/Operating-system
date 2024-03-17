#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string toHex(int value) {
    std::string hex = "0123456789ABCDEF";
    std::string result;
    while (value > 0) {
        result = hex[value % 16] + result;
        value /= 16;
    }
    // Добавляем "0" в начале, если меньше одной шестнадцатеричной цифры
    if (result.size() == 1) {
        result = "0" + result;
    }
    return result;
}

std::string replaceVowelsWithHex(const std::string& input) {
    std::string result;
    for (char c : input) {
        if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U') {
            // Заменяем гласные на их ASCII коды в формате "{0xDD}"
            result += "{0x" + toHex(static_cast<int>(c)) + "}";
        } else {
            result += c;
        }
    }
    return result;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " input_file output_file" << std::endl;
        return 1;
    }

    const char* input_file = argv[1];
    const char* output_file = argv[2];
    int size = 5000;

    // Создаем именованные каналы
    mkfifo("pipe1.fifo", 0666);
    mkfifo("pipe2.fifo", 0666);

    pid_t pid1, pid2;

    // Запускаем первый процесс (чтение из файла и запись в pipe1)
    pid1 = fork();

    if (pid1 == 0) {
        // В дочернем процессе (процесс 1)
        int fd1 = open("pipe1.fifo", O_WRONLY); // Открываем для записи
        std::ifstream infile(input_file);
        std::string line;

        while (std::getline(infile, line)) {
            // Пишем в pipe1
            write(fd1, line.c_str(), line.size());
        }

        // Закрываем именованный канал 1
        close(fd1);
        return 0;
    }

    // Запускаем второй процесс (чтение из pipe1, обработка, запись в pipe2)
    pid2 = fork();

    if (pid2 == 0) {
        // В дочернем процессе (процесс 2)
        int fd1 = open("pipe1.fifo", O_RDONLY); // Открываем для чтения
        int fd2 = open("pipe2.fifo", O_WRONLY); // Открываем для записи

        std::string input;
        char buffer[size];
        int bytes_read;

        while ((bytes_read = read(fd1, buffer, sizeof(buffer))) > 0) {
            input.append(buffer, bytes_read);
        }

        std::string replaced = replaceVowelsWithHex(input);

        // Пишем в pipe2
        write(fd2, replaced.c_str(), replaced.size());

        // Закрываем именованные каналы
        close(fd1);
        close(fd2);
        return 0;
    }

    // В родительском процессе (процесс 0)
    int status;
    waitpid(pid1, &status, 0);
    waitpid(pid2, &status, 0);

    // Читаем из pipe2 и записываем в файл
    int fd2 = open("pipe2.fifo", O_RDONLY); // Открываем для чтения
    std::ofstream outfile(output_file);

    char buffer[size];
    int bytes_read;

    while ((bytes_read = read(fd2, buffer, sizeof(buffer))) > 0) {
        // Пишем в файл
        outfile.write(buffer, bytes_read);
    }
    
    // Закрываем файл и именованный канал
    outfile.close();
    close(fd2);

    // Удаляем именованные каналы
    unlink("pipe1.fifo");
    unlink("pipe2.fifo");

    std::cout << "Processing complete. Output written to " << output_file << std::endl;

    return 0;
}
