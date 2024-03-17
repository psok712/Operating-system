#include <iostream>
#include <fstream>
#include <unistd.h>

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

    // Создаем неименованные каналы
    int pipe1[2]; // Для передачи данных от процесса 1 к процессу 2
    int pipe2[2]; // Для передачи данных от процесса 2 к процессу 3

    // Создаем неименованные каналы
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid1, pid2;

    // Запускаем первый процесс (чтение из файла и запись в pipe1)
    pid1 = fork();

    if (pid1 == 0) {
        // В дочернем процессе (процесс 1)
        close(pipe1[0]); // Закрываем чтение

        std::ifstream infile(input_file);
        std::string line;

        while (std::getline(infile, line)) {
            // Пишем в pipe1
            write(pipe1[1], line.c_str(), line.size());
        }

        // Закрываем запись в pipe1
        close(pipe1[1]);
        return 0;
    }

    // Запускаем второй процесс (чтение из pipe1, обработка, запись в pipe2)
    pid2 = fork();

    if (pid2 == 0) {
        // В дочернем процессе (процесс 2)
        close(pipe1[1]); // Закрываем запись в pipe1
        close(pipe2[0]); // Закрываем чтение из pipe2

        std::string input;
        char buffer[size];
        int bytes_read;

        while ((bytes_read = read(pipe1[0], buffer, sizeof(buffer))) > 0) {
            input.append(buffer, bytes_read);
        }

        std::string replaced = replaceVowelsWithHex(input);

        // Пишем в pipe2
        write(pipe2[1], replaced.c_str(), replaced.size());

        // Закрываем каналы
        close(pipe1[0]);
        close(pipe2[1]);
        return 0;
    }

    // В родительском процессе
    close(pipe1[0]); // Закрываем чтение из pipe1
    close(pipe1[1]); // Закрываем запись в pipe1

    // Запускаем третий процесс (чтение из pipe2 и запись в файл)
    pid_t pid3 = fork();

    if (pid3 == 0) {
        // В дочернем процессе (процесс 3)
        close(pipe2[1]); // Закрываем запись в pipe2

        // Открываем файл для записи
        std::ofstream outfile(output_file);

        char buffer[size];
        int bytes_read;

        while ((bytes_read = read(pipe2[0], buffer, sizeof(buffer))) > 0) {
            // Пишем в файл
            outfile.write(buffer, bytes_read);
        }
        
        // Закрываем файл и канал
        
        outfile.close();
        close(pipe2[0]);
        return 0;
    }

    // В родительском процессе
    close(pipe2[0]); // Закрываем чтение из pipe2
    close(pipe2[1]); // Закрываем запись в pipe2

    std::cout << "Processing complete. Output written to " << output_file << std::endl;

    return 0;
}

