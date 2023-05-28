#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CMD_LINE			4096
#define MAX_CMD_PARAMS			64
#define MAX_PATH                4096

static void parse_cmd_line(void);
static void proc_cd(void);

struct CMD {
    char *cmd_text;
    void (*proc)(void);
};


char g_cmd_line[MAX_CMD_LINE];

struct CMD g_cmds[] =
{
    {"cd", proc_cd},
    {NULL, NULL},
};


char *g_params[MAX_CMD_PARAMS];
int g_nparams;


void SimpleShell(void)
{
    char *str;
    pid_t pid;
    pid_t pid_wait;
    char cwd[MAX_PATH];
    int i;
    int status;

    for (;;)
    {
        if (getcwd(cwd, MAX_PATH) == NULL)
        {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        printf("%s>", cwd);
        fgets(g_cmd_line, MAX_CMD_LINE, stdin);

        if ((str = strchr(g_cmd_line, '\n')) != NULL)
        {
            *str = '\0';
        }

        parse_cmd_line();

        if (!g_nparams)
            continue;

        if (!strcmp(g_params[0], "exit"))
            break;

        for (i = 0; g_cmds[i].cmd_text != NULL; ++i)
        {
            if (!strcmp(g_cmds[i].cmd_text, g_params[0]))
            {
                g_cmds[i].proc();
                goto NEXT_CMD;
            }
        }

        if ((pid = fork()) == -1)
        {
            perror("fork");
        }

        if (!pid && execvp(g_params[0], &g_params[0]) == -1)
        {
            perror("execvp");
            exit(EXIT_FAILURE);
        }

        printf("create child process(id) = %lld\n", pid);

        if((pid_wait = waitpid(pid, &status, 0)) == -1)
        {
            perror("wait");
            exit(EXIT_FAILURE);
        }

        printf("parent wait child process(id) = %lld\n", pid_wait);

        if(WIFEXITED(status))
        {
            printf("child exit code: %d\n", WIFEXITED(status));
        }
        else
        {
            printf("exit abnormally");
        }

//        if (wait(NULL) == -1)
//        {
//            perror("wait");
//            exit(EXIT_FAILURE);
//        }
NEXT_CMD:
        ;
    }
}

static void parse_cmd_line(void)
{
    char *str;

    g_nparams = 0;

    for (str = strtok(g_cmd_line, " \t"); str != NULL; str = strtok(NULL, " \t"))
    {
        g_params[g_nparams++] = str;
    }

    g_params[g_nparams] = NULL;
}

static void proc_cd(void)
{
    char *env;

    if (g_nparams == 1)
    {
        if ((env = getenv("HOME")) == NULL)
        {
            perror("getenv");
            return;
        }

        if (chdir(env) == -1)
        {
            perror("chdir");
        }

        return;
    }

    if (g_nparams > 2)
    {
        fprintf(stderr, "too many arguments!..\n");
        return;
    }

    if (chdir(g_params[1]) == -1)
        perror("chdir");
}
