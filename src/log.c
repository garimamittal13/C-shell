#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define MAX_LOG_SIZE 15
#define LOG_FILE_PATH ".shell_log"

char *command_log[MAX_LOG_SIZE];
int log_count = 0;

// Function to load log from file
void load_log() {
    FILE *file = fopen(LOG_FILE_PATH, "r");
    if (!file) return;

    char line[1024];
    while (fgets(line, sizeof(line), file) && log_count < MAX_LOG_SIZE) {
        line[strcspn(line, "\n")] = 0;  // Remove newline character
        command_log[log_count] = strdup(line);
        log_count++;
    }

    fclose(file);
}

// Function to save log to file
void save_log() {
    FILE *file = fopen(LOG_FILE_PATH, "w");
    if (!file) return;

    for (int i = 0; i < log_count && i < MAX_LOG_SIZE; i++) {
        fprintf(file, "%s\n", command_log[i]);
    }

    fclose(file);
}

// Function to add a command to the log
void add_to_log(char *command) {
    // Skip duplicate consecutive commands
    if (log_count > 0 && strcmp(command_log[(log_count - 1) % MAX_LOG_SIZE], command) == 0) {
        return;
    }

    if (log_count < MAX_LOG_SIZE) {
        command_log[log_count] = strdup(command);
    } else {
        free(command_log[log_count % MAX_LOG_SIZE]); // Overwrite the oldest command
        command_log[log_count % MAX_LOG_SIZE] = strdup(command);
    }

    log_count++;
    save_log();  // Save the log to the file after adding a new command
}

// Function to print the log
void print_log() {
    for (int i = 0; i < log_count && i < MAX_LOG_SIZE; i++) {
        printf("%d: %s\n", i + 1, command_log[i]);
    }
}

// Function to purge the log
void purge_log() {
    for (int i = 0; i < log_count && i < MAX_LOG_SIZE; i++) {
        free(command_log[i]);
        command_log[i] = NULL;
    }
    log_count = 0;
    save_log();
}

// Function to execute a command from the log
void execute_log_command(int index) {
    if (index < 1 || index > log_count || index > MAX_LOG_SIZE) {
        printf("Invalid log index\n");
        return;
    }

    char *command = command_log[(index - 1) % MAX_LOG_SIZE];
    printf("Executing: %s\n", command);
    execute_command(command, 0); // Call the execute_command function instead of system()
}
