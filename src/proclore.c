#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "proclore.h"

void procloreCommand(char *pid_str) {
    pid_t pid;
    if (pid_str == NULL) {
        pid = getpid(); // Default to current shell's process ID
    } else {
        pid = atoi(pid_str); // Convert provided PID string to integer
    }

    char path[1024], buffer[4096];
    int process_group_id;
    int virtual_memory_size;
    FILE *file;

    // Display the process ID
    printf("PID: %d\n", pid);

    // Get the process status
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file) {
        fscanf(file, "%*d %*s %c", &buffer[0]); // Read the status
        char *status = buffer[0] == 'R' ? "R (Running)" :
                       buffer[0] == 'S' ? "S (Sleeping)" :
                       buffer[0] == 'Z' ? "Z (Zombie)" :
                       "Unknown";
        printf("Process Status: %s\n", status);
        fclose(file);
    } else {
        perror("Error reading process status");
        return;
    }

    // Get the process group ID
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    file = fopen(path, "r");
    if (file) {
        fscanf(file, "%*d %*s %*c %*d %*d %d", &process_group_id); // Read the process group ID
        printf("Process Group: %d\n", process_group_id);
        fclose(file);
    } else {
        perror("Error reading process group");
        return;
    }

    // Get the virtual memory size
    snprintf(path, sizeof(path), "/proc/%d/statm", pid);
    file = fopen(path, "r");
    if (file) {
        fscanf(file, "%d", &virtual_memory_size); // Read the virtual memory size
        printf("Virtual Memory: %d kB\n", virtual_memory_size * 4); // Assuming page size is 4kB
        fclose(file);
    } else {
        perror("Error reading virtual memory");
        return;
    }

    // Get the executable path
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    int len = readlink(path, buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0'; // Null-terminate the path
        printf("Executable Path: %s\n", buffer);
    } else {
        perror("Error reading executable path");
    }
}
