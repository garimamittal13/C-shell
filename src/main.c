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
        displayPrompt();

        char input[1024];

        // Read user input
        if (fgets(input, sizeof(input), stdin) != NULL)
        {
            // Remove the newline character at the end of the input string
            size_t len = strlen(input);
            if (len > 0 && input[len - 1] == '\n')
            {
                input[len - 1] = '\0';
            }
             // Add command to log
            check_background_processes(); // Check for background process completion
            // Handle input
            handle_input(input);
        }
        else
        {
            // Handle EOF (Ctrl-D)
            if (feof(stdin))
            {
                printf("\n");
                // Kill all background processes before exiting
                for (int i = 0; i < bg_count; i++)
                {
                    kill(bg_processes[i].pid, SIGKILL);
                }
                exit(0);
            }
            else
            {
                perror("fgets");
                break;
            }
        }
    }
    return 0;
}
