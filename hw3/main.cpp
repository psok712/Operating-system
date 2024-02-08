#include <iostream>
#include <unistd.h>
#include <cstdint>

int main() {
    pid_t pid, ppid, chpid;
    size_t number;
    std::cout << "Enter positive number: ";
    std::cin >> number;
    chpid = fork();
    pid = getpid();
    ppid = getppid();

    if (!chpid) { // child
        size_t result = 1;
        bool flag = false;

        while (number > 0) {
            if (UINT64_MAX / number < result) {
                flag = true;
                break;
            }
            result *= number;
            --number;
        }

        if (!flag) {
            printf("I'm child. Result = %zu\n", result);
            printf("Child info: pid = %d, ppid = %d, chpid = %d\n", (int)pid, (int)ppid, (int)chpid);
        } else {
            printf("I'm child. Result incorrect (out of range size_t)\n");
            printf("Child info: pid = %d, ppid = %d, chpid = %d\n", (int)pid, (int)ppid, (int)chpid);
        }
    } else {
        size_t a = 0;
        size_t b = 1;
        bool flag = false;

        while (number != 0) {
            if (UINT64_MAX - b < a) {
                flag = true;
                break;
            }
            auto c = a + b;
            a = b;
            b = c;
            --number;
        }

        if (!flag) {
            printf("I'm parent. Result = %zu\n", b);
            printf("Parent info: pid = %d, ppid = %d, chpid = %d\n", (int)pid, (int)ppid, (int)chpid);
        } else {
            printf("I'm parent. Result incorrect (out of range size_t)\n");
            printf("Parent info: pid = %d, ppid = %d, chpid = %d\n", (int)pid, (int)ppid, (int)chpid);
        }

        system("ls");
    }


    return 0;
}
