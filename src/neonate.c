#include "neonate.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>
#include <sys/stat.h>

// Function to check if a key is pressed without blocking
int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    // Save old terminal settings
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    // Set terminal to non-canonical mode, no echo
    newt.c_lflag &= ~(ICANON | ECHO);
    // Apply new settings
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    // Set non-blocking read
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    // Try to read character
    ch = getchar();
    if (ch != EOF) {
        // Put character back into stdin
        ungetc(ch, stdin);

        // Restore settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, oldf);
        return 1;
    }
    // Restore settings
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    return 0;
}

// Function to find the PID of the most recently created process
pid_t get_most_recent_pid() {
    DIR *proc = opendir("/proc");
    if (proc == NULL) {
        perror("opendir");
        return -1;
    }

    struct dirent *entry;
    pid_t recent_pid = -1;
    unsigned long long latest_starttime = 0;

    while ((entry = readdir(proc)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // Check if the directory name is a number
            pid_t pid = atoi(entry->d_name);
            if (pid > 0) {
                // Read /proc/[pid]/stat
                char stat_path[256];
                snprintf(stat_path, sizeof(stat_path), "/proc/%d/stat", pid);
                FILE *stat_file = fopen(stat_path, "r");
                if (stat_file) {
                    // Read the content
                    char buffer[1024];
                    if (fgets(buffer, sizeof(buffer), stat_file) != NULL) {
                        // Tokenize to get the 22nd field
                        char *token;
                        int field = 1;
                        unsigned long long starttime = 0;
                        token = strtok(buffer, " ");
                        while (token != NULL) {
                            if (field == 22) {
                                starttime = atoll(token);
                                break;
                            }
                            field++;
                            token = strtok(NULL, " ");
                        }
                        if (starttime > latest_starttime) {
                            latest_starttime = starttime;
                            recent_pid = pid;
                        }
                    }
                    fclose(stat_file);
                }
            }
        }
    }
    closedir(proc);
    return recent_pid;
}

void neonateCommand(char *args[]) {
    if (args[1] == NULL || args[2] == NULL || strcmp(args[1], "-n") != 0) {
        fprintf(stderr, "Usage: neonate -n [time_arg]\n");
        return;
    }
    int time_arg = atoi(args[2]);
    if (time_arg <= 0) {
        fprintf(stderr, "Invalid time argument\n");
        return;
    }
    printf("Press 'x' to stop\n");
    while (1) {
        pid_t recent_pid = get_most_recent_pid();
        if (recent_pid != -1) {
            printf("%d\n", recent_pid);
        } else {
            printf("Could not retrieve recent PID\n");
        }
        // Wait for time_arg seconds or until 'x' is pressed
        time_t start_time = time(NULL);
        while (1) {
            if (kbhit()) {
                char ch = getchar();
                if (ch == 'x' || ch == 'X') {
                    return;
                }
            }
            if (difftime(time(NULL), start_time) >= time_arg) {
                break;
            }
            usleep(100000); // Sleep for 0.1 seconds to reduce CPU usage
        }
    }
}
