#include "commandhandler.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h> // Include for signal handling
#include <errno.h>  // Include for error handling
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include "color.h"
#include "log.h"
#include "myshrc.h"
#include <time.h>
#include "proclore.h"
#include "seek.h"
#include "neonate.h"
#include "iman.h"
#include <pwd.h>
#include <grp.h>
#include <termios.h>

#define PATH_MAX 4096
#define MAX_PIPE_COMMANDS 10
BackgroundProcess bg_processes[100];
int bg_count = 0;

// Global variables for process group management
extern pid_t shell_pgid;
extern pid_t fg_pid;

// Function to add a background process to the tracking list
void add_background_process(pid_t pid, char *command, int is_running)
{
    bg_processes[bg_count].pid = pid;
    strncpy(bg_processes[bg_count].command, command, sizeof(bg_processes[bg_count].command) - 1);
    bg_processes[bg_count].command[sizeof(bg_processes[bg_count].command) - 1] = '\0';
    bg_processes[bg_count].is_running = is_running;
    bg_count++;
}

// Function to check the status of background processes
void check_background_processes()
{
    pid_t pid;
    int status;

    // Use waitpid with WNOHANG to check for background process termination
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < bg_count; i++)
        {
            if (bg_processes[i].pid == pid)
            {
                if (WIFEXITED(status))
                {
                    printf("Background process %s (%d) exited normally\n", bg_processes[i].command, pid);
                }
                else if (WIFSIGNALED(status))
                {
                    printf("Background process %s (%d) terminated by signal %d\n", bg_processes[i].command, pid, WTERMSIG(status));
                }
                // Shift remaining background processes forward to remove the finished one
                for (int j = i; j < bg_count - 1; j++)
                {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
                break;
            }
        }
    }
}

// Function to compare processes by command name for sorting
int compare_processes(const void *a, const void *b)
{
    const BackgroundProcess *procA = (const BackgroundProcess *)a;
    const BackgroundProcess *procB = (const BackgroundProcess *)b;
    return strcmp(procA->command, procB->command);
}

// Modify fgCommand to handle process groups and terminal control
void fgCommand(char *args[])
{
    if (args[1] == NULL)
    {
        printf("Usage: fg <pid>\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    int found = 0;

    for (int i = 0; i < bg_count; i++)
    {
        if (bg_processes[i].pid == pid)
        {
            found = 1;

            // Bring the process to the foreground
            fg_pid = pid;
            printf("Bringing process %d to foreground\n", pid);

            // Send the continue signal if the process is stopped
            if (!bg_processes[i].is_running)
            {
                kill(pid, SIGCONT);
            }

            // Set the child's process group ID
            setpgid(pid, pid);

            // Set the terminal's foreground process group to the child
            tcsetpgrp(STDIN_FILENO, pid);

            // Wait for the process to finish
            int status;
            do
            {
                waitpid(pid, &status, WUNTRACED);
                if (WIFSTOPPED(status))
                {
                    printf("\nProcess with PID %d stopped\n", pid);
                    bg_processes[i].is_running = 0;
                    break;
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));

            fg_pid = -1; // Reset foreground process ID

            // Restore the shell as the foreground process group
            tcsetpgrp(STDIN_FILENO, shell_pgid);

            if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                // Remove process from background tracking if it's done
                for (int j = i; j < bg_count - 1; j++)
                {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
            }

            return;
        }
    }

    if (!found)
    {
        printf("No such process found\n");
    }
}

// Function to resume a stopped job in the background
void bgCommand(char *args[])
{
    if (args[1] == NULL)
    {
        printf("Usage: bg <pid>\n");
        return;
    }

    pid_t pid = atoi(args[1]);
    int found = 0;

    printf("Attempting to find process %d\n", pid); // Debugging

    // Loop through the list of background processes
    for (int i = 0; i < bg_count; i++)
    {
        printf("Checking process %d\n", bg_processes[i].pid); // Debugging

        if (bg_processes[i].pid == pid)
        {
            found = 1;
            printf("Found process %d\n", pid); // Debugging log

            // Check if the process is stopped
            if (!bg_processes[i].is_running)
            {
                printf("Process %d is stopped, resuming it\n", pid);

                // Resume the stopped process in the background
                if (kill(pid, SIGCONT) == -1)
                {
                    perror("Error resuming process");
                }
                else
                {
                    bg_processes[i].is_running = 1;
                    printf("Process %d resumed successfully\n", pid); // Debugging log
                }
            }
            else
            {
                printf("Process %d is already running\n", pid); // Debugging log
            }

            return;
        }
    }

    if (!found)
    {
        printf("No such process found\n");
    }
}

// Function to handle the activities command
void activitiesCommand()
{
    qsort(bg_processes, bg_count, sizeof(BackgroundProcess), compare_processes);
    // Print processes in the required format
    for (int i = 0; i < bg_count; i++)
    {
        // Check if the process is still running or stopped
        int status;
        pid_t result = waitpid(bg_processes[i].pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == 0)
        {
            // Process has not changed state
        }
        else if (result > 0)
        {
            if (WIFSTOPPED(status))
            {
                bg_processes[i].is_running = 0;
            }
            else if (WIFCONTINUED(status))
            {
                bg_processes[i].is_running = 1;
            }
            else if (WIFEXITED(status) || WIFSIGNALED(status))
            {
                // Process has terminated; remove from list
                for (int j = i; j < bg_count - 1; j++)
                {
                    bg_processes[j] = bg_processes[j + 1];
                }
                bg_count--;
                i--; // Adjust index after removal
                continue;
            }
        }

        char *state = bg_processes[i].is_running ? "Running" : "Stopped";
        printf("%d : %s - %s\n", bg_processes[i].pid, bg_processes[i].command, state);
    }
}

// Function to determine if a file is executable
int is_executable(struct stat *fileStat)
{
    return (fileStat->st_mode & S_IXUSR) || (fileStat->st_mode & S_IXGRP) || (fileStat->st_mode & S_IXOTH);
}

int handle_redirection(char *args[], int *input_fd, int *output_fd)
{
    int i = 0, j = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL)
        {
            *input_fd = open(args[i + 1], O_RDONLY);
            if (*input_fd < 0)
            {
                perror("open input");
                return -1;
            }
            i += 2; // Skip '<' and filename
        }
        else if ((strcmp(args[i], ">") == 0 || strcmp(args[i], ">>") == 0) && args[i + 1] != NULL)
        {
            int flags = O_WRONLY | O_CREAT;
            if (strcmp(args[i], ">>") == 0)
                flags |= O_APPEND;
            else
                flags |= O_TRUNC;

            *output_fd = open(args[i + 1], flags, 0644);
            if (*output_fd < 0)
            {
                perror("open output");
                return -1;
            }
            i += 2; // Skip '>' or '>>' and filename
        }
        else
        {
            args[j++] = args[i++]; // Copy normal args
        }
    }
    args[j] = NULL; // NULL-terminate cleaned args[]
    return 0;
}

// Function to handle the reveal command
void revealCommand(char *args[])
{
    int show_all = 0;
    int long_format = 0;
    char *path = ".";
    // Parse flags
    for (int i = 1; args[i] != NULL; i++)
    {
        if (args[i][0] == '-')
        {
            if (strchr(args[i], 'a'))
                show_all = 1;
            if (strchr(args[i], 'l'))
                long_format = 1;
        }
        else
        {
            path = args[i];
        }
    }
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        perror("reveal");
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!show_all && entry->d_name[0] == '.')
            continue; // Skip hidden files unless -a is set

        struct stat fileStat;
        char fullPath[PATH_MAX];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        if (stat(fullPath, &fileStat) == -1)
        {
            perror("stat");
            continue;
        }
        if (long_format)
        {
            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" ");
            // Number of links
            printf("%ld ", fileStat.st_nlink);
            // Owner and group
            struct passwd *pw = getpwuid(fileStat.st_uid);
            struct group *gr = getgrgid(fileStat.st_gid);
            printf("%s %s ", pw->pw_name, gr->gr_name);
            // File size
            printf("%ld ", fileStat.st_size);
            // Last modified time
            char timebuf[80];
            struct tm *timeinfo = localtime(&fileStat.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo);
            printf("%s ", timebuf);
        }
        // File name with color coding
        if (S_ISDIR(fileStat.st_mode))
        {
            print_colored(entry->d_name, "dir");
        }
        else if (is_executable(&fileStat))
        {
            print_colored(entry->d_name, "exe");
        }
        else
        {
            print_colored(entry->d_name, "file");
        }

        if (long_format)
        {
            printf("\n"); // Newline for long format
        }
        else
        {
            printf(" "); // Space for short format
        }
    }
    if (!long_format)
    {
        printf("\n"); // Final newline for short format
    }
    closedir(dir);
}

// Function to handle the hop command
void hopCommand(char *args[])
{
    char cwd[PATH_MAX];
    char *home_dir = getenv("HOME");
    for (int i = 1; args[i] != NULL; i++)
    {
        char *dir = args[i];
        // Handle special cases
        if (strcmp(dir, "~") == 0)
        {
            dir = home_dir;
        }
        else if (strcmp(dir, "-") == 0)
        {
            dir = getenv("OLDPWD");
            if (!dir)
            {
                perror("OLDPWD not set");
                continue;
            }
        }
        // Change directory
        if (chdir(dir) == -1)
        {
            perror("hop");
            continue;
        }
        // Update OLDPWD and PWD
        setenv("OLDPWD", getenv("PWD"), 1);
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            setenv("PWD", cwd, 1);
        }
        else
        {
            perror("getcwd");
        }
    }
    // If no argument is provided, hop to the home directory
    if (args[1] == NULL)
    {
        if (chdir(home_dir) == -1)
        {
            perror("hop");
        }
        else
        {
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                setenv("PWD", cwd, 1);
            }
            else
            {
                perror("getcwd");
            }
        }
    }
}

// Function to handle the mk_hop command
void mk_hop(char *dir)
{
    char mkdir_command[256];
    snprintf(mkdir_command, sizeof(mkdir_command), "mkdir %s", dir);
    execute_command(mkdir_command, 0); // Create directory

    char *hop_args[] = {"hop", dir, NULL};
    hopCommand(hop_args); // Hop into the directory
}

// Function to handle the hop_seek command
void hop_seek(char *dir)
{
    char *hop_args[] = {"hop", dir, NULL};
    hopCommand(hop_args); // Hop into the directory

    char *seek_args[] = {"seek", dir, NULL};
    seekCommand(seek_args); // Search for files/directories with the same name
}

void execute_pipe_with_redirection(char *segments[MAX_PIPE_COMMANDS], int num_cmds)
{
    int pipefds[2 * (num_cmds - 1)];
    pid_t pids[MAX_PIPE_COMMANDS];

    for (int i = 0; i < num_cmds - 1; i++)
    {
        if (pipe(pipefds + 2 * i) < 0)
        {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_cmds; i++)
    {
        int input_fd = -1, output_fd = -1;
        char *args[MAX_ARG];

        // Tokenize and handle redirection
        char *token = strtok(segments[i], " \t\n");
        int arg_idx = 0;
        while (token && arg_idx < MAX_ARG - 1)
        {
            args[arg_idx++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_idx] = NULL;

        if (handle_redirection(args, &input_fd, &output_fd) != 0)
            continue;

        pids[i] = fork();
        if (pids[i] == 0)
        {
            // CHILD
            if (input_fd != -1)
            {
                dup2(input_fd, STDIN_FILENO);
                close(input_fd);
            }
            else if (i > 0)
            {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }

            if (output_fd != -1)
            {
                dup2(output_fd, STDOUT_FILENO);
                close(output_fd);
            }
            else if (i < num_cmds - 1)
            {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_cmds - 1); j++)
                close(pipefds[j]);

            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pids[i] < 0)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (input_fd != -1)
            close(input_fd);
        if (output_fd != -1)
            close(output_fd);
    }

    for (int i = 0; i < 2 * (num_cmds - 1); i++)
        close(pipefds[i]);

    for (int i = 0; i < num_cmds; i++)
        waitpid(pids[i], NULL, 0);
}

// Function to execute a command with redirection and pipes
void execute_command_with_redirection(char *cmd)
{
    char *segments[MAX_PIPE_COMMANDS];
    int segment_count = 0;

    // Split the full command by pipe
    char *segment = strtok(cmd, "|");
    while (segment != NULL && segment_count < MAX_PIPE_COMMANDS)
    {
        segments[segment_count++] = segment;
        segment = strtok(NULL, "|");
    }

    if (segment_count > 1)
    {
        // Multiple pipe segments
        execute_pipe_with_redirection(segments, segment_count);
    }
    else
    {
        // Single command, handle redirection normally
        char *args[MAX_ARG];
        int arg_idx = 0;
        char *token = strtok(cmd, " \t\n");
        while (token && arg_idx < MAX_ARG - 1)
        {
            args[arg_idx++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_idx] = NULL;

        int input_fd = -1, output_fd = -1;
        if (handle_redirection(args, &input_fd, &output_fd) == 0)
        {
            pid_t pid = fork();
            if (pid == 0)
            {
                if (input_fd != -1)
                {
                    dup2(input_fd, STDIN_FILENO);
                    close(input_fd);
                }
                if (output_fd != -1)
                {
                    dup2(output_fd, STDOUT_FILENO);
                    close(output_fd);
                }
                execvp(args[0], args);
                perror("execvp");
                exit(EXIT_FAILURE);
            }
            else if (pid > 0)
            {
                waitpid(pid, NULL, 0);
            }
            else
            {
                perror("fork");
            }
        }
    }
}

// Global variable to keep track of foreground process ID
extern pid_t fg_pid;

// Function to handle the ping command
void pingCommand(char *args[])
{
    if (args[1] == NULL || args[2] == NULL)
    {
        fprintf(stderr, "Usage: ping <pid> <signal_number>\n");
        return;
    }
    pid_t pid = atoi(args[1]);
    int signal_number = atoi(args[2]) % 32; // Modulo 32 as per specification
    // Check if the process exists
    if (kill(pid, 0) == -1)
    {
        if (errno == ESRCH)
        {
            printf("No such process found\n");
        }
        else
        {
            perror("ping");
        }
        return;
    }
    // Send the signal
    if (kill(pid, signal_number) == -1)
    {
        perror("ping");
    }
    else
    {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    }
}

void execute_command(char *cmd, int background)
{
    // Handle commands with pipes or I/O redirection
    if (strchr(cmd, '|') || strchr(cmd, '<') || strchr(cmd, '>'))
    {
        execute_command_with_redirection(cmd);
        return;
    }
    pid_t pid;
    int status;
    char *argv[MAX_ARG];
    int argc = 0;
    time_t start_time, end_time;
    // Tokenize command by spaces
    char full_command[1024];
    strncpy(full_command, cmd, sizeof(full_command) - 1);
    full_command[sizeof(full_command) - 1] = '\0';
    // Tokenize the command
    char *token = strtok(full_command, " \t\n");
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL; // Null-terminate the argument list
    if (argc == 0)
        return; // Empty command
        
    if (strcmp(argv[0], "fg") == 0) {
        fgCommand(argv);
        return;
    }
    if (strcmp(argv[0], "bg") == 0) {
        bgCommand(argv);
        return;
    }
    if (strcmp(argv[0], "log") == 0)
    {
        if (argc == 1)
        {
            print_log();
        }
        else if (argc == 2 && strcmp(argv[1], "purge") == 0)
        {
            purge_log();
        }
        else if (argc == 3 && strcmp(argv[1], "execute") == 0)
        {
            int index = atoi(argv[2]);
            execute_log_command(index);
        }
        return;
    }

    // Handle 'exit' command
    if (strcmp(argv[0], "exit") == 0)
    {
        exit(0);
    }

    // Handle 'mk_hop' command
    if (strcmp(argv[0], "mk_hop") == 0 && argc == 2)
    {
        mk_hop(argv[1]);
        return;
    }

    // Handle 'hop_seek' command
    if (strcmp(argv[0], "hop_seek") == 0 && argc == 2)
    {
        hop_seek(argv[1]);
        return;
    }

    // Handle 'hop' command
    if (strcmp(argv[0], "hop") == 0)
    {
        hopCommand(argv);
        return;
    }

    // Handle 'reveal' command
    if (strcmp(argv[0], "reveal") == 0)
    {
        revealCommand(argv);
        return;
    }

    // Handle 'proclore' command
    if (strcmp(argv[0], "proclore") == 0)
    {
        if (argc == 1)
        {
            procloreCommand(NULL); // No PID provided, use current process
        }
        else if (argc == 2)
        {
            procloreCommand(argv[1]); // PID provided
        }
        else
        {
            printf("Usage: proclore [pid]\n");
        }
        return;
    }

    // Handle 'seek' command
    if (strcmp(argv[0], "seek") == 0)
    {
        seekCommand(argv);
        return;
    }

    // Handle 'activities' command
    if (strcmp(argv[0], "activities") == 0)
    {
        activitiesCommand();
        return;
    }

    // Handle 'ping' command
    if (strcmp(argv[0], "ping") == 0)
    {
        pingCommand(argv);
        return;
    }

    // Handle 'neonate' command
    if (strcmp(argv[0], "neonate") == 0)
    {
        neonateCommand(argv);
        return;
    }

    // Handle 'iMan' command
    if (strcmp(argv[0], "man") == 0)
    {
        // printf("Recognized iMan command\n"); // Debug statement
        imanCommand(argv);
        return;
    }

    // Fork a child process to execute the non-custom command
    pid = fork();
    if (pid == 0)
    {
        // Set the child process group ID to its own PID
        setpgid(0, 0);
        // If foreground process, take control of the terminal
        if (!background)
        {
            tcsetpgrp(STDIN_FILENO, getpid());
        }
        // Restore default signal handlers
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        if (execvp(argv[0], argv) == -1)
        {
            fprintf(stderr, "ERROR: '%s' is not a valid command\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else if (pid > 0)
    {
        // Set the child's process group ID
        setpgid(pid, pid);
        if (!background)
        {
            // Set the terminal's foreground process group to the child
            tcsetpgrp(STDIN_FILENO, pid);
            fg_pid = pid;            // Set the foreground process ID
            start_time = time(NULL); // Record start time
            // Wait for the child process to finish or stop
            do
            {
                waitpid(pid, &status, WUNTRACED);
                if (WIFSTOPPED(status))
                {
                    printf("\nProcess with PID %d stopped\n", pid);
                    add_background_process(pid, cmd, 0); // Mark as stopped
                    break;
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            fg_pid = -1; // Reset foreground process ID
            // Restore the shell as the foreground process group
            tcsetpgrp(STDIN_FILENO, shell_pgid);
            end_time = time(NULL); // Record end time

            // Calculate and print elapsed time if more than 2 seconds
            if (difftime(end_time, start_time) > 2)
            {
                printf("Foreground process '%s' took %.0f seconds\n", argv[0], difftime(end_time, start_time));
            }
        }
        else
        {
            // Background process
            printf("Background process started with PID %d\n", pid);
            printf("[%d] %d\n", getpid(), pid);  // Print the child PID
            add_background_process(pid, cmd, 1); // Add background process to tracking list
        }
    }
    else
    {
        perror("fork");
    }
}
