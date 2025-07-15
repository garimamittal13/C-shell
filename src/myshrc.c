#include "myshrc.h"
#include "commandhandler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALIAS_MAX 100

typedef struct {
    char alias[50];
    char command[100];
} Alias;

Alias alias_table[ALIAS_MAX];
int alias_count = 0;

void add_alias(const char *alias, const char *command) {
    if (alias_count < ALIAS_MAX) {
        strcpy(alias_table[alias_count].alias, alias);
        strcpy(alias_table[alias_count].command, command);
        alias_count++;
    }
}

const char* resolve_alias(const char *input) {
    for (int i = 0; i < alias_count; i++) {
        if (strcmp(alias_table[i].alias, input) == 0) {
            return alias_table[i].command;
        }
    }
    return input;
}

void load_myshrc() {
    FILE *file = fopen("myshrc", "r");
    if (!file) {
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *pos;
        if ((pos = strchr(line, '\n')) != NULL) {
            *pos = '\0';
        }

        if (line[0] == '#') continue;

        if (strncmp(line, "alias ", 6) == 0 || strchr(line, '=') != NULL) {
            char *alias_name = strtok(line, "=");
            char *command = strtok(NULL, "=");
            if (alias_name && command) {
                add_alias(alias_name + (strncmp(line, "alias ", 6) == 0 ? 6 : 0), command);
            }
        }
    }

    fclose(file);
}
