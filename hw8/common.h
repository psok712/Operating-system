// Заголовочный файл, содержащий общие данные для писателей и читателей
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <signal.h>

#define BUF_SIZE 10     // Размер буфера ячеек
// Структура для хранения в разделяемой памяти буфера и необходимых данных
// Задает буфер, доступный процессам
typedef struct {
  int store[BUF_SIZE];  // буфер для заполнения ячеек
  int next_write;       // индекс для записи в буфер
  int next_read;        // индекс для чтения из буфера
  int have_reader;      // индикатор наличия читателя
  int reader_pid;       // идентификатор процесса читателя
  int writer_pid;       // идентификатор процесса писателя
} shared_memory;

// имя области разделяемой памяти
extern const char* shar_object ;
extern int buf_id;        // дескриптор объекта памяти
extern shared_memory *buffer;    // указатель на разделямую память, хранящую буфер

// имя семафора для занятых ячеек
extern const char *full_sem_name;
extern sem_t *full;   // указатель на семафор занятых ячеек

// имя семафора для свободных ячеек
extern const char *empty_sem_name;
extern sem_t *empty;   // указатель на семафор свободных ячеек

// имя семафора (мьютекса) для критической секции,
// обеспечивающей доступ писателя к буферу
extern const char *mutex_write_sem_name;
extern sem_t *write_mutex;   // указатель на семафор писателей

// имя семафора (мьютекса) для критической секции,
// обеспечивающей доступ писателя к буферу
extern const char *mutex_read_sem_name;
extern sem_t *read_mutex;   // указатель на семафор читателей

// Имя семафора для управления доступом.
// Позволяет читателю дождаться разрешения от читателя.
// Даже если читатель стартовал первым
extern const char *admin_sem_name;
extern sem_t *admin;   // указатель на семафор читателей

// Функция осуществляющая при запуске общие манипуляции с памятью и семафорами
// для децентрализованно подключаемых процессов читателей и писателей.
void init(void);

// Функция закрывающая семафоры общие для писателей и читателей
void close_common_semaphores(void);

// Функция, удаляющая все семафоры и разделяемую память
void unlink_all(void);