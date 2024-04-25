#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>

#define SPECTATORS_PAINTING 5
#define COUNT_PAINTINGS 5
#define MAX_WAITING_TIME 7
#define MAX_VISITORS 25
#define MAX_NEW_VISITORS 300

struct SharedData {
    int countSpectators[COUNT_PAINTINGS];
    int countVisitors;
    sem_t mutex;
    sem_t cond;
};

void visitor(int id, struct SharedData *sharedData) {
    printf("Visitor %d entered the gallery.\n", id);
    for (int i = 0; i < COUNT_PAINTINGS; ++i) {
        sem_wait(&sharedData->mutex);
        printf("Visitor %d is at Painting %d.\n", id, i + 1);
        while (sharedData->countSpectators[i] >= SPECTATORS_PAINTING) {
            printf("Painting %d is crowded. Visitor %d is waiting.\n", i + 1, id);
            sem_post(&sharedData->mutex);
            sem_wait(&sharedData->cond);
            sem_wait(&sharedData->mutex);
        }
        ++sharedData->countSpectators[i];
        ++sharedData->countVisitors;
        printf("Visitor %d is watching Painting %d. Total viewers: %d\n", id, i + 1, sharedData->countSpectators[i]);
        sem_post(&sharedData->mutex);

        sleep(rand() % MAX_WAITING_TIME + 1);

        sem_wait(&sharedData->mutex);
        --sharedData->countVisitors;
        --sharedData->countSpectators[i];
        printf("Visitor %d finished watching Painting %d. Total visitors: %d\n", id, i + 1, sharedData->countVisitors);
        sem_post(&sharedData->cond);
        sem_post(&sharedData->mutex);
    }

    printf("Visitor %d has finished viewing all paintings and left the gallery.\n", id);
    exit(0); // После просмотра всех картин посетитель покидает галерею
}

void watchman(struct SharedData *sharedData) {
    int newVisitors = 0;
    while (1) {
        sem_wait(&sharedData->mutex);
        printf("Current visitors: %d\n", sharedData->countVisitors);
        if (sharedData->countVisitors < MAX_VISITORS && newVisitors < MAX_NEW_VISITORS) {
            sem_post(&sharedData->mutex);
            ++sharedData->countVisitors;
            ++newVisitors;
            printf("New visitor arrived. Total visitors: %d\n", sharedData->countVisitors);
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "Fork failed\n");
                exit(1);
            } else if (pid == 0) {
                visitor(getpid(), sharedData);
                exit(0);
            }
        } else {
            sem_post(&sharedData->mutex);
            printf("Gallery is full. New visitors have to wait outside.\n");
            sleep(1); // Sleep for a short time to avoid busy-waiting
        }
    }
}

int main() {
    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        exit(1);
    }
    ftruncate(shm_fd, sizeof(struct SharedData));
    struct SharedData *sharedData = mmap(NULL, sizeof(struct SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sharedData == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Инициализация семафоров в разделяемой памяти
    if (sem_init(&sharedData->mutex, 1, 1) == -1) {
        perror("sem_init mutex");
        exit(1);
    }
    if (sem_init(&sharedData->cond, 1, 0) == -1) {
        perror("sem_init cond");
        exit(1);
    }

    sharedData->countVisitors = 0;

    srand(time(NULL)); // Initialize random seed

    pid_t pid = fork();
    if (pid < 0) {
        fprintf(stderr, "Fork failed\n");
        exit(1);
    } else if (pid == 0) {
        watchman(sharedData);
        exit(0);
    }

    wait(NULL);

    sem_destroy(&sharedData->mutex);
    sem_destroy(&sharedData->cond);
    shm_unlink("/shared_memory");

    return 0;
}
