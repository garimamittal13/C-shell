#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "inputhandler.h"
#include "commandhandler.h"
#include "prompt.h"
#include "log.h"
#include "myshrc.h"
#include <termios.h>
#include <readline/readline.h>
#include <readline/history.h>


pid_t shell_pid;
pid_t shell_pgid;
pid_t fg_pid = -1; // Foreground process ID
char home_dir[PATH_MAX] = "";
// Signal handler for SIGINT (Ctrl-C)
void sigint_handler(int sig)
{
    if (fg_pid != -1)
    {
        kill(-fg_pid, SIGINT); // Send SIGINT to the foreground process group
    }
    else
    {
        printf("\n");
        displayPrompt();
        fflush(stdout);
    }
}

// Signal handler for SIGTSTP (Ctrl-Z)
void sigtstp_handler(int sig)
{
    if (fg_pid != -1)
    {
        kill(-fg_pid, SIGTSTP); // Send SIGTSTP to the foreground process group
    }
    else
    {
        printf("\n");
        displayPrompt();
        fflush(stdout);
    }
}

int main()
{
    shell_pid = getpid();

    // Set the shell process group
    shell_pgid = shell_pid;
    if (setpgid(shell_pid, shell_pgid) < 0)
    {
        perror("Failed to set shell process group");
        exit(1);
    }

    // Take control of the terminal
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    // Ignore signals in the shell
    signal(SIGTTOU, SIG_IGN);

    // Set up signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    load_myshrc(); // Load aliases and functions from .myshrc
    load_log();    // Load the log from the file

    while (1)
    {
        if (getcwd(home_dir, sizeof(home_dir)) == NULL)
        {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
        // Build prompt string
        char prompt_str[512];
        snprintf(prompt_str, sizeof(prompt_str), "<%s@localhost:%s> ", getenv("USER"), home_dir);

        // Read user input with readline (supports history and tab completion)
        char *input = readline(prompt_str);

        if (input == NULL)
        {
            // Handle EOF (Ctrl-D)
            printf("\n");
            // Kill all background processes before exiting
            for (int i = 0; i < bg_count; i++)
            {
                kill(bg_processes[i].pid, SIGKILL);
            }
            exit(0);
        }

        // Add non-empty commands to history and process them
        if (strlen(input) > 0)
        {
            add_history(input);  // Add to readline history for arrow key navigation
            check_background_processes();  // Check for background process completion
            handle_input(input);  // Process the command
        }

        free(input);  // readline() allocates memory that must be freed
    }
    return 0;
}
