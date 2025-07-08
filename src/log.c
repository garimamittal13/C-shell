#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

#define MAX_LOG_SIZE 15
#define LOG_FILE_PATH ".shell_log"

// circular buffer state
static char *command_log[MAX_LOG_SIZE];
static int   log_start = 0;   // index of oldest entry
static int   log_count = 0;   // number of valid entries

// ——— Load the existing log into our buffer ———
void load_log(void) {
    FILE *file = fopen(LOG_FILE_PATH, "r");
    if (!file) return;

    // clear any old in-memory entries
    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % MAX_LOG_SIZE;
        free(command_log[idx]);
        command_log[idx] = NULL;
    }
    log_start = log_count = 0;

    char line[1024];
    while (fgets(line, sizeof(line), file) && log_count < MAX_LOG_SIZE) {
        line[strcspn(line, "\n")] = '\0';
        command_log[log_count++] = strdup(line);
    }

    fclose(file);
}

// ——— Save the buffer back to disk in correct order ———
void save_log(void) {
    FILE *file = fopen(LOG_FILE_PATH, "w");
    if (!file) return;

    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % MAX_LOG_SIZE;
        fprintf(file, "%s\n", command_log[idx]);
    }

    fclose(file);
}

// ——— Append a new command, evicting the oldest if full ———
void add_to_log(char *command) {
    // skip duplicate consecutive
    if (log_count > 0) {
        int last = (log_start + log_count - 1) % MAX_LOG_SIZE;
        if (strcmp(command_log[last], command) == 0)
            return;
    }

    int idx;
    if (log_count < MAX_LOG_SIZE) {
        // room at the tail
        idx = (log_start + log_count) % MAX_LOG_SIZE;
        log_count++;
    } else {
        // buffer full → overwrite oldest
        idx = log_start;
        free(command_log[idx]);
        log_start = (log_start + 1) % MAX_LOG_SIZE;
    }

    command_log[idx] = strdup(command);
    save_log();
}

// ——— Print the in-memory history in order ———
void print_log(void) {
    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % MAX_LOG_SIZE;
        printf("%2d: %s\n", i + 1, command_log[idx]);
    }
}

// ——— Free everything & clear both memory and disk ———
void purge_log(void) {
    for (int i = 0; i < log_count; i++) {
        int idx = (log_start + i) % MAX_LOG_SIZE;
        free(command_log[idx]);
        command_log[idx] = NULL;
    }
    log_start = log_count = 0;
    save_log();
}

// ——— Execute a saved entry by 1-based index ———
void execute_log_command(int index) {
    if (index < 1 || index > log_count) {
        printf("Invalid log index\n");
        return;
    }
    int idx = (log_start + index - 1) % MAX_LOG_SIZE;
    printf("Executing: %s\n", command_log[idx]);
    execute_command(command_log[idx], 0);
}
