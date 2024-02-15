#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) {
    const int SIZE = 8;
    int f_read, f_write;
    struct stat file_mod;
    char buffer[SIZE];
    size_t read_bytes;

    if (std::atoi(argv[1]) != 2) {
        printf("Incorrect input!\n");
        return 0;
    }

    if (stat(argv[2], &file_mod) < 0) {
        printf("Unknown erroe :(");
        return 0;
    }

    f_read = open(argv[2], O_RDONLY);
    if (f_read < 0) {
        printf("Could not open file for reading!\n");
        return 0;
    }

    f_write = open(argv[3], O_WRONLY | O_CREAT, file_mod.st_mode);
    if (f_write < 0) {
        printf("Could not open file for writing!\n");
        return 0;
    }

    do {
        read_bytes = read(f_read, buffer, SIZE);
        write(f_write, buffer, read_bytes);
    } while (read_bytes == SIZE);

    if (close(f_read) < 0) {
        printf("Could not close file for reading!\n");
        return 0;
    }

    if (close(f_write) < 0) {
        printf("Could not close file for writing!\n");
        return 0;
    }

    return 0;
}