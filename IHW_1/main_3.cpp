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

    int pipe1[2]; // Канал для передачи данных от процесса 1 к процессу 2

    // Создаем неименованный канал
    if (pipe(pipe1) == -1) {
        perror("Pipe creation failed");
        return 1;
    }

    pid_t pid;

    // Запускаем процесс
    pid = fork();

    if (pid == 0) {
        // В дочернем процессе (процесс 1)

        // Закрываем неиспользуемый конец канала
        close(pipe1[0]); // Закрываем чтение из pipe1

        // Открываем файл для чтения
        std::ifstream infile(input_file);
        std::string line;
        std::string input;

        while (std::getline(infile, line)) {
            input += line + "\n"; // Считываем все строки в одну строку
        }

        // Пишем в pipe1
        write(pipe1[1], input.c_str(), input.size());

        // Закрываем конец записи в pipe1
        close(pipe1[1]);

        return 0;
    } else if (pid > 0) {
        // В родительском процессе (процесс 2)

        // Закрываем неиспользуемый конец канала
        close(pipe1[1]); // Закрываем запись в pipe1

        // Читаем из pipe1
        std::string input;
        char buffer[size];
        int bytes_read;

        while ((bytes_read = read(pipe1[0], buffer, sizeof(buffer))) > 0) {
            input.append(buffer, bytes_read);
        }

        // Заменяем гласные на шестнадцатеричные коды
        std::string replaced = replaceVowelsWithHex(input);

        // Создаем неименованный канал для обратной передачи
        int pipe2[2]; // Канал для передачи данных от процесса 2 к процессу 1
        if (pipe(pipe2) == -1) {
            perror("Pipe creation failed");
            return 1;
        }

        pid_t pid2;
        pid2 = fork();

        if (pid2 == 0) {
            // В дочернем процессе второго процесса (процесс 3)

            // Закрываем неиспользуемый конец канала
            close(pipe2[1]); // Закрываем запись в pipe2

            // Читаем из pipe2 и записываем в файл
            std::ofstream outfile(output_file);
            char buffer2[size];
            int bytes_read2;

            while ((bytes_read2 = read(pipe2[0], buffer2, sizeof(buffer2))) > 0) {
                // Пишем в файл
                outfile.write(buffer2, bytes_read2);
            }

            // Закрываем файл и канал
            outfile.close();
            close(pipe2[0]);

            return 0;
        } else if (pid2 > 0) {
            // В родительском процессе второго процесса (процесс 2)

            // Закрываем неиспользуемый конец канала
            close(pipe2[0]); // Закрываем чтение из pipe2

            // Пишем в pipe2
            write(pipe2[1], replaced.c_str(), replaced.size());

            // Закрываем конец записи в pipe2
            close(pipe2[1]);

            return 0;
        }
    }

    std::cout << "Processing complete. Output written to " << output_file << std::endl;

    return 0;
}
