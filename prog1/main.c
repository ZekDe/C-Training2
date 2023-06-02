#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>


#define MAX_MSG_SIZE        8192

struct SHARED_OBJECT {
    pthread_mutex_t mutex;
    int counter;
};

void exit_sys(const char *format, ...);
void printerr(const char *format, ...);

void pipe_Example(void); // PosixTraining
void shared_memory_Example(void); // PosixTraining
void message_queue_Example(void); // PosixTraining
void mutex_Example2(void); // PosixTraining
void semaphore_consumer_Example(void); // PosixTraining


int main(int argc, char *argv[])
{
    //pipe_Example();
    //shared_memory_Example();
    //message_queue_Example();
    //mutex_Example2();
    semaphore_consumer_Example();

    return 0;
}

void pipe_Example(void)
{
    int fd;
    int val;
    ssize_t result;

    if ((fd = open("mypipe", O_RDONLY)) == -1)
        exit_sys("open");

    getchar();

    while ((result = read(fd, &val, sizeof(int))) > 0)
        printf("%d ", val), fflush(stdout);

    printf("\n");

    if (result == -1)
        exit_sys("read");

    close(fd);
}

void shared_memory_Example(void)
{
    int fdshm;
    void *shmaddr;
    char *str;

    if ((fdshm = shm_open("/sample_shared_memory_name", O_RDWR, 0/*no O_CREAT no argument*/)) == -1)
        exit_sys("shm_open");

    if ((shmaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fdshm, 0)) == MAP_FAILED)
        exit_sys("mmap");

    str = (char *)shmaddr;

    printf("press ENTER to read...\n");
    getchar();

    puts(str);


    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fdshm);
}

void message_queue_Example(void)
{
    mqd_t mq;
    struct mq_attr attr;
    char msg[MAX_MSG_SIZE];
    unsigned int prio;
    ssize_t result;

    if ((mq = mq_open("/sample_message_queue", O_RDONLY)) == -1)
        exit_sys("mq_open");

    for (;;) {
        printf("Message Queue Attibute\n");
        if (mq_getattr(mq, &attr) == -1)
            exit_sys("mq_getattr");

        printf("mq_flags: %ld\n", attr.mq_flags);
        printf("mq_maxmsg: %ld\n", attr.mq_maxmsg);
        printf("mq_msgsize: %ld\n", attr.mq_msgsize);
        printf("mq_curmsgs: %ld\n", attr.mq_curmsgs);
        printf("-----------------------\n");

        printf("Press ENTER to receive message..\n");
        getchar();

        if ((result = mq_receive(mq, msg, MAX_MSG_SIZE, &prio)) == -1)
            exit_sys("mq_receive");
        if (!strcmp(msg, "quit"))
            break;
        printf("-----------------------\n");
        printf("%lld bytes received with priority %u: \"%s\"\n", (long long)result, prio, msg);
        printf("-----------------------\n");
    }

    mq_close(mq);

    // when the communication is done, delete it
    if (mq_unlink("/sample_message_queue") == -1)
        exit_sys("mq_unlink");
}

void mutex_Example2(void)
{
    int fdshm;
    void* shmaddr;
    struct SHARED_OBJECT* so;
    int i;

    if ((fdshm = shm_open("/sample_shared_memory_name", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
        exit_sys("shm_open");

    if ((shmaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fdshm, 0)) == MAP_FAILED)
        exit_sys("mmap");

    so = (struct SHARED_OBJECT*)shmaddr;

    printf("Press ENTER to continue...\n");
    getchar();
    printf("Entering loop...\n");

    for (i = 0; i < 100000000; ++i) {
        pthread_mutex_lock(&so->mutex);
        ++so->counter;
        pthread_mutex_unlock(&so->mutex);
    }

    printf("Press ENTER to exit...\n");
    getchar();

    printf("Counter: %d\n", so->counter);

    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fdshm);
}

void semaphore_consumer_Example(void)
{
    int fdshm;
    void *shmaddr;
    int *pshared;
    sem_t *sem_producer;
    sem_t *sem_consumer;
    int val;

    if ((fdshm = shm_open("/interprocess-producer-consumer-shared-memory", O_RDONLY, 0)) == -1)
        exit_sys("shm_open");

    if ((shmaddr = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fdshm, 0)) == MAP_FAILED)
        exit_sys("mmap");

    pshared = (int *)shmaddr;

    if ((sem_producer = sem_open("/interprocess-producer-consumer-producer-semaphore", O_WRONLY)) == NULL)
        exit_sys("sem_open");

    if ((sem_consumer = sem_open("/interprocess-producer-consumer-consumer-semaphore", O_RDONLY)) == NULL)
        exit_sys("sem_open");

    for (;;) {
        usleep(rand() % 300000);
        sem_wait(sem_consumer);
        val = *pshared;
        sem_post(sem_producer);
        printf("%d ", val), fflush(stdout);
        if (val == 99)
            break;
    }
    putchar('\n');

    sem_destroy(sem_consumer);
    sem_destroy(sem_producer);

    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fdshm);
}

void printerr(const char *format, ...)
{
    va_list va;

    va_start(va, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, va);

    va_end(va);
}


void exit_sys(const char *format, ...)
{
    va_list va;

    va_start(va, format);

    vfprintf(stderr, format, va);
    fprintf(stderr, ": %s", strerror(errno));

    va_end(va);

    exit(EXIT_FAILURE);
}
