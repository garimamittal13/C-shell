#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>

#define MAX_ARG 100
#define MAX_PIPE_COMMANDS 10
#define SHELL_MAX_INPUT 1024 

#ifndef WCONTINUED
#define WCONTINUED 0x0008
#endif


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

extern char home_dir[PATH_MAX];

typedef struct {
    pid_t pid;
    char command[256];
    int is_running; // 1 for running, 0 for stopped
} BackgroundProcess;

extern BackgroundProcess bg_processes[100];
extern int bg_count;

void execute_command(char *cmd, int background);
void execute_pipe_with_redirection(char *segments[MAX_PIPE_COMMANDS], int num_cmds);
void execute_command_with_redirection(char *cmd);
void hopCommand(char *args[]);
void check_background_processes();
int is_executable(struct stat* fileStat);
int handle_redirection(char *args[], int *input_fd, int *output_fd);
void activitiesCommand();
void procloreCommand(char *pid);
void revealCommand(char *args[]);
void mk_hop(char *dir);
void hop_seek(char *dir);
void pingCommand(char *args[]);
void fgCommand(char *args[]);
void bgCommand(char *args[]);


#endif
