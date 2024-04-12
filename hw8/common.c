// Общий модуль, осуществляющий одинаковые административные функции
// как для писателя, так и для читателя.
#include "common.h"
#include <semaphore.h>

// имя области разделяемой памяти
const char *shar_object = "/posix-shar-object";
int buf_id;        // дескриптор объекта памяти
shared_memory *buffer;    // указатель на разделямую память, хранящую буфер

// имя семафора для занятых ячеек
const char *full_sem_name = "/full-semaphore";
sem_t *full;   // указатель на семафор занятых ячеек

// имя семафора для свободных ячеек
const char *empty_sem_name = "/empty-semaphore";
sem_t *empty;   // указатель на семафор свободных ячеек

// имя семафора (мьютекса) для чтения данных из буфера
const char *mutex_read_sem_name = "/mutex-read-semaphore";
sem_t *read_mutex;   // указатель на семафор читателей

// имя семафора (мьютекса) для записи данных в буфер
const char *mutex_write_sem_name = "/mutex-write-semaphore";
sem_t *write_mutex;   // указатель на семафор писателей

// Имя семафора для подсчета контроля за запуском процессов
const char *admin_sem_name = "/admin-semaphore";
sem_t *admin;   // указатель на семафор читателей

// Функция осуществляющая при запуске общие манипуляции с памятью и семафорами
// для децентрализованно подключаемых процессов читателей и писателей.
void init(void) {
  // Создание или открытие семафора для администрирования (доступ открыт)
  if ((admin = sem_open(admin_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create admin semaphore");
    exit(-1);
  }
  // Создание или открытие мьютекса для доступа писателей к буферу (доступ открыт)
  if ((write_mutex = sem_open(mutex_write_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: Can not create mutex semaphore");
    exit(-1);
  }
  // Создание или открытие мьютекса для доступа читателей к буферу (доступ открыт)
  if ((read_mutex = sem_open(mutex_read_sem_name, O_CREAT, 0666, 1)) == 0) {
    perror("sem_open: Can not create mutex semaphore");
    exit(-1);
  }
  // Количество свободных ячеек равно BUF_SIZE
  if ((empty = sem_open(empty_sem_name, O_CREAT, 0666, BUF_SIZE)) == 0) {
    perror("sem_open: Can not create free semaphore");
    exit(-1);
  };
  // Количество занятых ячеек равно 0
  if ((full = sem_open(full_sem_name, O_CREAT, 0666, 0)) == 0) {
    perror("sem_open: Can not create busy semaphore");
    exit(-1);
  };
}

// Функция закрывающая семафоры общие для писателей и читателей
void close_common_semaphores(void) {
  if (sem_close(empty) == -1) {
    perror("sem_close: Incorrect close of empty semaphore");
    exit(-1);
  };
  if (sem_close(full) == -1) {
    perror("sem_close: Incorrect close of busy semaphore");
    exit(-1);
  };
  if (sem_close(admin) == -1) {
    perror("sem_close: Incorrect close of admin semaphore");
    exit(-1);
  };
  if (sem_close(write_mutex) == -1) {
    perror("sem_close: Incorrect close of mutex semaphore");
    exit(-1);
  };
  if (sem_close(read_mutex) == -1) {
    perror("sem_close: Incorrect close of mutex semaphore");
    exit(-1);
  };
}

// Функция, удаляющая все семафоры и разделяемую память
void unlink_all(void) {
  if (sem_unlink(mutex_write_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of mutex semaphore");
    // exit(-1);
  };
  if (sem_unlink(mutex_read_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of mutex semaphore");
    // exit(-1);
  };
  if (sem_unlink(empty_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of empty semaphore");
    // exit(-1);
  };
  if (sem_unlink(full_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of full semaphore");
    // exit(-1);
  };
  if (sem_unlink(admin_sem_name) == -1) {
    perror("sem_unlink: Incorrect unlink of admin semaphore");
    // exit(-1);
  };

  // удаление разделяемой памяти
  if (shm_unlink(shar_object) == -1) {
    perror("shm_unlink");
    // exit(-1);
  }
}