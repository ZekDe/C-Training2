#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include "simpleshell.h"
#include <sys/mman.h>
#include <mqueue.h>
#include <pthread.h>
#include <time.h>

#define MAX_MSG_SIZE        8192

void clear_stdin_buffer(void);
void printerr(const char *format, ...);
void exit_thread(const char *msg, int result);
void exit_sys(const char *format, ...);


void SimpleFork_Example(void);
void CreateProcess_Example(void);
void CreateProcess1_Example(void);
void CreateProcessWithArgs_Example(int argc, char *argv[]);
void CreateProcessWithArgs_Example2(int argc, char *argv[]);
void SimpleShell_Example(void);
void dup_Example(void);
void dup2_rout_Example(void);
void redirect_Example(int argc, char *argv[]);
void pipe_Example(void);
void pipe_bash_Example(int argc, char *argv[]);
void pipe_Example2(int argc, char *argv[]);
void pipe_Example3(void);
void shared_memory_Example(void);
void memory_map_file_Example(void);
void message_queue_Example(void);
void thread_Example(void);
void mutex_Example(void);
void mutex_Example2(void); // prog1



int main(int argc, char *argv[])
{
    //SimpleFork_Example();
    //CreateProcess_Example();
    //CreateProcess1_Example();
    //CreateProcessWithArgs_Example(argc, argv);
    //CreateProcessWithArgs2_Example(argc, argv);
    //SimpleShell_Example();
    //dup_Example();
    //dup2_rout_Example();
    //redirect_Example(argc, argv);
    //pipe_Example();
    //pipe_bash_Example(argc, argv);
    //pipe_Example2(argc, argv);
    //pipe_Example3(); // prog1
    //shared_memory_Example(); // prog1
    //memory_map_file_Example();
    //message_queue_Example(); // prog1
    //thread_Example();
    //mutex_Example();
    mutex_Example2();


    return 0;
}


void SimpleFork_Example(void)
{
    static int val = 0;
    pid_t pid;

    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(pid != 0)
    {
        //printf("parent process...\n");
        val = 100;
        sleep(10);
    }
    else
    {
        sleep(5);
        //printf("child process...\n");
        printf("val = %d\n", val);
    }

    printf("common code...\n");
}

void CreateProcess_Example(void)
{
    pid_t pid;


    printf("program running...");

    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(!pid && execl("/bin/ls", "/bin/ls", "-l", "-i", (char*)NULL) == -1)
    {
        exit_sys("execl"); // terminate the child process
    }

    for(int i = 0; i < 10; ++i) // parent
    {
        printf("%d\n", i);
        sleep(1);
    }
}

void CreateProcess1_Example(void)
{
    pid_t pid;
    char *args[] = {"/bin/ls", "-l", "-i", NULL};

    printf("program running...");

    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(!pid && execv("/bin/ls", args) == -1)
    {
        exit_sys("execv"); // terminate the child process
    }

    for(int i = 0; i < 10; ++i) // parent reaches here
    {
        printf("%d\n", i);
        sleep(1);
    }
}

void CreateProcessWithArgs_Example(int argc, char *argv[])
{
    pid_t pid;

    if(argc < 2)
    {
        fprintf(stderr, "what are you doing???\n");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(!pid && execv(argv[1], &argv[1]) == 1)
    {
        exit_sys("execv"); // terminate the child process
    }

    if(wait(NULL) == -1)
    {
        exit_sys("wait");
    }
}

void CreateProcessWithArgs2_Example(int argc, char *argv[])
{
    pid_t pid;

    if(argc < 2)
    {
        fprintf(stderr, "wrong arg number??\n");
        exit(EXIT_FAILURE);
    }

    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(!pid && execvp(argv[1], &argv[1]) == 1) // search in path if no "/" character
    {
        exit_sys("execv"); // terminate the child process
    }

    if(wait(NULL) == -1)
    {
        exit_sys("wait");
    }
}

void SimpleShell_Example(void)
{
    SimpleShell();
}

void dup_Example(void)
{
    int fd1;
    int fd2;
    char buf[25] = {0};

    ssize_t n;

    if((fd1 = open("test.txt", O_RDONLY)) == -1)
    {
        exit_sys("open");
    }
    if((fd2 = dup(fd1)) == -1)
    {
        exit_sys("dup");
    }


    if((n = read(fd1, buf, 10)) == -1)
    {
        exit_sys("read");
    }

    printf("%s\n", buf);


    if((n = read(fd2, buf, 10)) == -1)
    {
        exit_sys("read");
    }

    printf("%s\n", buf);

    close(fd1);
    close(fd2);
    // now, file descriptor counter is 0
}

void dup2_rout_Example(void)
{
    int fd;
    if((fd = open("test.txt", O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1)
    {
        exit_sys("open");
    }
    if(dup2(fd, 1) == -1) // route to stdout (atomic)
    {
        exit_sys("dup2");
    }
    close(fd);

    for(int i = 0; i < 10; ++i)
    {
        printf("%d\n", i);
    }

}

void redirect_Example(int argc, char *argv[])
{
    char *dirpos;
    char *progargs[512], *str, *path;
    int i;
    pid_t pid;
    int fd;

    if(argc != 2)
    {
        fprintf(stderr, "less arg");
    }

    if((dirpos = strchr(argv[1],'>')) == NULL)
    {
        exit_sys("> not found\n");
    }

    *dirpos = '\0';

    i = 0;
    for(str = strtok(argv[1], " \t"); str != NULL; str = strtok(NULL, " \t"))
    {
        progargs[i++] = str;
    }
    progargs[i] = NULL;

    if((path = strtok(dirpos + 1, " \t")) == NULL)
    {
        exit_sys("connot found file dir\n");
    }

    if((strtok(NULL, " \t")) != NULL)
    {
        exit_sys("too many path\n");
    }

    //duplicate process control block(PCB)), file descriptor and code. Both process point the same objects in file desc.
    if((pid = fork()) == -1)
    {
        exit_sys("fork");
    }

    if(!pid)
    {
        // child area
        if((fd = open(path, O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1)
        {
            exit_sys("open");//terminate child proc
        }
        if(dup2(fd, 1) == -1)
        {
            exit_sys("dup2");
        }
        close(fd);
        //time to exec, remove the current code and run command in bash. Now, in file descriptor, stdout points the path.
        if(execvp(progargs[0], &progargs[0]) == -1)
        {
            exit_sys("execcvp");
        }
        //unreachable code
    }
    // child connot reach here, parent area
    if(waitpid(pid, NULL, 0) == -1) // parent waits child
    {
        exit_sys("waitpid");
    }

}

void pipe_Example(void)
{
    int pfds[2];
    pid_t pid;
    int val;
    ssize_t res;

    if(pipe(pfds) == -1)
    {exit_sys("pipe");}

    if((pid = fork()) == -1)
    {exit_sys("fork");}

    if(pid != 0) // parent
    {
        close(pfds[0]);// [0] read

        for(int i = 0; i < 100; ++i)
        {
            if(write(pfds[1], &i, sizeof(int)) == -1)
            {exit_sys("write");}
        }

        close(pfds[1]);

        if(waitpid(pid, NULL, 0) == -1)
        {exit_sys("waitpid");}
    }
    else // child
    {
        close(pfds[1]); // [1] write

        while((res = read(pfds[0], &val, sizeof(int))) > 0)
        {
            printf("%d ", val);
            fflush(stdout);
        }

        if(-1 == res)
        {exit_sys("read");}

        printf("\n");

        close(pfds[0]);
    }
}

void pipe_bash_Example(int argc, char *argv[])
{
    char* ppos;
    char *args_out[512], *args_in[512];
    char* str;
    int pfds[2];
    pid_t pid_out, pid_in;
    int i;

    if (argc != 2)
    {
        printerr( "wrong number of arguments!..\n");
        exit(EXIT_FAILURE);
    }

    if ((ppos = strchr(argv[1], '|')) == NULL) {
        printerr("no pipe symbol!..\n");
        exit(EXIT_FAILURE);
    }

    *ppos = '\0';

    i = 0;
    // before '|' character
    for (str = strtok(argv[1], " \t"); str != NULL; str = strtok(NULL, " \t"))
    {
        args_out[i++] = str;
    }
    args_out[i] = NULL;

    //after '|' character ,ppos+1 points it
    i = 0;
    for (str = strtok(ppos + 1, " \t"); str != NULL; str = strtok(NULL, " \t"))
    {
        args_in[i++] = str;
    }
    args_in[i] = NULL;

    if (pipe(pfds) == -1)
        exit_sys("pipe");

    if ((pid_out = fork()) == -1)
        exit_sys("fork");

    // RUN left
    if (pid_out == 0)
    {
        // ROUTE  child's stdout to pipe(write)
        if (dup2(pfds[1], 1) == -1)
            exit_sys("dup2");

        close(pfds[0]);
        close(pfds[1]);

        if (execvp(args_out[0], &args_out[0]) == -1)
            exit_sys("execvp");

        /* unreachable code */
    }

    if ((pid_in = fork()) == -1)
        exit_sys("fork");

    // RUN right
    if (pid_in == 0)
    {
        if (dup2(pfds[0], 0) == -1)
            exit_sys("dup2");

        close(pfds[0]);
        close(pfds[1]);

        if (execvp(args_in[0], &args_in[0]) == -1)
            exit_sys("execvp");

        /* unreachable code */
    }

    close(pfds[0]);
    close(pfds[1]);

    if (waitpid(pid_out, NULL, 0) == -1)
        exit_sys("waitpid");

    if (waitpid(pid_in, NULL, 0) == -1)
        exit_sys("waitpid");
}

void pipe_Example2(int argc, char *argv[])
{
    int result;
    int m_flag;
    char *m_arg;
    int err_flag;
    int mode;
    int i;

    umask(0); // every parameters will be used which is entered by user for this process.

    opterr = 0;

    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    err_flag = 0;
    m_flag = 0;

    while ((result = getopt(argc, argv, "m:")) != -1)
    {
        switch (result)
        {
        case 'm':
            m_flag = 1;
            m_arg = optarg;
            break;

        case '?':
            if (optopt == 'm')
                printerr("-m option must have an argument!..\n");
            else
                printerr("invalid option: -%c\n", optopt);

            err_flag = 1;
            break;

        default:
            break;
        }
    }

    if (err_flag)
        exit(EXIT_FAILURE);

    if (m_flag)
        sscanf(m_arg, "%o", &mode);

    // any argumant is not exist
    if (optind == argc)
    {
        printerr( "at least one path must be specified!..\n");
        exit(EXIT_FAILURE);
    }

    for (i = optind; i < argc; ++i)
    {
        if (mkfifo(argv[i], mode) == -1) // 0 byte
            exit_sys("mkfifo");
    }

}

void pipe_Example3(void)
{
    int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    int fd;

    if (mkfifo("mypipe", mode) == -1)
        exit_sys("mkfifo");

    if ((fd = open("mypipe", O_WRONLY)) == -1)
        exit_sys("open");

    for (int i = 0; i < 100; ++i)
        if (write(fd, &i, sizeof(int)) == -1)
            exit_sys("write");

    close(fd);

}

void shared_memory_Example(void)
{
    int fdshm;
    void *shmaddr;
    char *str;

    if ((fdshm = shm_open("/sample_shared_memory_name", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
        exit_sys("shm_open");

    if (ftruncate(fdshm, 4096) == -1)
        exit_sys("ftruncate");


    if ((shmaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fdshm, 0)) == MAP_FAILED)
        exit_sys("mmap");

    str = (char *)shmaddr;

    strcpy(str, "This is a test...");

    printf("Press ENTER to exit...\n");
    getchar();

    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fdshm);

    /* shm_open objects are permanent until reboot */
    //shm_unlink("/sample_shared_memory_name"); delete it immediately
    if (shm_unlink("/sample_shared_memory_name") == -1)
        exit_sys("shm_unlink");
}

void memory_map_file_Example(void)
{
    int fd;
    void *shmaddr;
    char *str;

    if ((fd = open("test.txt", O_RDWR)) == -1)
        exit_sys("open");

    if ((shmaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
        exit_sys("mmap");

    str = (char *)shmaddr;

    printf("press ENTER to read...\n");
    getchar();

    puts(str);

    memcpy(shmaddr, "yyy", 3);
    msync(shmaddr, 4096, MS_SYNC); // it is offered by posix standarts to perform changes in file

    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fd);
}

void message_queue_Example(void)
{
    struct mq_attr attr;
    mqd_t mq;
    char msg[MAX_MSG_SIZE], *str;
    unsigned int prio;

    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;

    if ((mq = mq_open("/sample_message_queue", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH, &attr)) == -1)
        exit_sys("mq_open");

    for (;;) {
        printf("Message: ");
        fflush(stdout);

        fgets(msg, MAX_MSG_SIZE, stdin);

        if ((str = strchr(msg, '\n')) != NULL)
            *str = '\0';

        printf("Priority: ");
        fflush(stdout);
        scanf("%u", &prio/*NULL means is no need priorty info*/);
        clear_stdin_buffer();

        if (mq_send(mq, msg, strlen(msg) + 1/* +1 is NULL character */, prio) == -1)
            exit_sys("mq_send");

        if (!strcmp(msg, "quit")) // when the consumer receive the message then quit
            break;
    }

    mq_close(mq);
}

void *thread_proc1(void *param);
void *thread_proc2(void *param);

void thread_Example(void)
{
    pthread_t tid1;
    pthread_t tid2;
    void *tret;
    int result;

    if ((result = pthread_create(&tid1, NULL, thread_proc1, NULL)) != 0)
        exit_thread("pthread_create", result);
    if ((result = pthread_create(&tid2, NULL, thread_proc2, NULL)) != 0)
        exit_thread("pthread_create", result);

    for (int i = 0; i < 3; ++i)
    {
        printf("main thread: %d\n", i);
        sleep(1);
    }

    printf("counter completed\n");

    if ((result = pthread_join(tid1, &tret)) != 0)
        exit_thread("pthread_join", result);

    printf("Thread-2 exited with %ld\n", (long)tret);

    printf("press ENTER to exit..\n");
    getchar();
}

void *thread_proc1(void *param)
{
    int i;

    for (i = 0; i < 10; ++i) {
        printf("thread1: %d\n", i);
        sleep(1);
    }

    return (void*)100;
}

void *thread_proc2(void *param)
{
    int i;

    for (i = 0; i < 5; ++i) {
        printf("thread2: %d\n", i);
        sleep(1);
    }

    return (void*)200;
}

void* thread_proc3(void* param);
void* thread_proc4(void* param);
void do_machine(const char* name);

pthread_mutex_t g_mutex;

void mutex_Example(void)
{
    int result;
    pthread_t tid1, tid2;

    srand(time(NULL));

    // To prevent self lock, recursive mutex needed

    if ((result = pthread_mutex_init(&g_mutex, NULL)) != 0)
        exit_thread("pthread_mutex_init", result);

    if ((result = pthread_create(&tid1, NULL, thread_proc3, NULL)) != 0)
        exit_thread("pthread_create", result);

    if ((result = pthread_create(&tid2, NULL, thread_proc4, NULL)) != 0)
        exit_thread("pthread_create", result);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&g_mutex);
}

void* thread_proc3(void* param)
{
    int i;

    for (i = 0; i < 10; ++i)
        do_machine("thread-1");

    return NULL;
}

void* thread_proc4(void* param)
{
    int i;

    for (i = 0; i < 10; ++i)
        do_machine("thread-2");

    return NULL;
}

void do_machine(const char* name)
{
    pthread_mutex_lock(&g_mutex);

    printf("---------------\n");
    printf("1) %s\n", name);
    usleep(rand() % 300000);
    printf("2) %s\n", name);
    usleep(rand() % 300000);
    printf("3) %s\n", name);
    usleep(rand() % 300000);
    printf("4) %s\n", name);
    usleep(rand() % 300000);
    printf("5) %s\n", name);
    usleep(rand() % 300000);

    pthread_mutex_unlock(&g_mutex);
}


struct SHARED_OBJECT {
    pthread_mutex_t mutex;
    int counter;
};

void mutex_Example2(void)
{
    int fdshm;
    int result;
    void* shmaddr;
    pthread_mutexattr_t mattr;
    struct SHARED_OBJECT* so;
    int i;

    if ((fdshm = shm_open("/sample_shared_memory_name", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1)
        exit_sys("shm_open");

    if (ftruncate(fdshm, 4096) == -1)
        exit_sys("ftruncate");

    if ((shmaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fdshm, 0)) == MAP_FAILED)
        exit_sys("mmap");

    so = (struct SHARED_OBJECT*)shmaddr;

    if ((result = pthread_mutexattr_init(&mattr)) != 0)
        exit_thread("pthread_mutexattr_init", result);

    if ((result = pthread_mutexattr_setpshared(&mattr, 1)) != 0)
        exit_thread("pthread_mutexattr_setpshared", result);

    if ((result = pthread_mutex_init(&so->mutex, &mattr)) != 0)
        exit_thread("pthread_mutexattr_setpshared", result);

    pthread_mutexattr_destroy(&mattr);

    printf("Press ENTER to continue...\n");
    getchar();
    printf("Entering loop...\n");

    for (i = 0; i < 10000000; ++i) {
        pthread_mutex_lock(&so->mutex);
        ++so->counter;
        pthread_mutex_unlock(&so->mutex);
    }

    printf("Press ENTER to exit...\n");
    getchar();

    printf("Counter: %d\n", so->counter);

    pthread_mutex_destroy(&so->mutex);

    // must be in order to terminate: shmaddr -> fdshm
    if (munmap(shmaddr, 4096) == -1)
        exit_sys("munmap");

    close(fdshm);

    if (shm_unlink("/sample_shared_memory_name") == -1) // terminate the shared-memory-obj
        exit_sys("shm_unlink");
}


void clear_stdin_buffer(void)
{
    int ch;

    while ((ch = getchar()) != EOF && ch != '\n')
        ;
}

void printerr(const char *format, ...)
{
    va_list va;

    va_start(va, format);

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, va);

    va_end(va);
}

void exit_thread(const char *msg, int result)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(result));

    exit(EXIT_FAILURE);
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
