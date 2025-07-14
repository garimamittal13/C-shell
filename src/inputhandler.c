#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "inputhandler.h"
#include "commandhandler.h"
#include "log.h"
#include "myshrc.h"

#define MAX_ARG 100

// Function to determine if the input contains pipes or redirection
int contains_special_symbols(char *cmd) {
    return strchr(cmd, '|') || strchr(cmd, '<') || strchr(cmd, '>'); 
}

void handle_input(char *input) {
    add_to_log(input);
    char *semi_cmds[MAX_ARG];
    int semi_count = 0;

    // Split by `;` first
    char *semi_token = strtok(input, ";");
    while (semi_token) {
        semi_cmds[semi_count++] = semi_token;
        semi_token = strtok(NULL, ";");
    }

    for (int i = 0; i < semi_count; ++i) {
        char *cmd_group = semi_cmds[i];

        // Check for multiple commands split by `&`
        int bg_split = 0;
        for (int k = 0; cmd_group[k]; k++) {
            if (cmd_group[k] == '&' && cmd_group[k + 1] != '\0') {
                bg_split = 1;
                break;
            }
        }

        if (bg_split) {
            // Split by '&' for multiple commands
            char *amp_cmds[MAX_ARG];
            int amp_count = 0;
            char *amp_token = strtok(cmd_group, "&");
            while (amp_token) {
                amp_cmds[amp_count++] = amp_token;
                amp_token = strtok(NULL, "&");
            }

            for (int j = 0; j < amp_count; ++j) {
                char *cmd = amp_cmds[j];
                while (*cmd == ' ' || *cmd == '\t') cmd++;

                // Strip trailing whitespace
                char *end = cmd + strlen(cmd) - 1;
                while (end > cmd && (*end == ' ' || *end == '\t' || *end == '\n'))
                    *end-- = '\0';

                if (*cmd == '\0') continue;

                cmd = (char *)resolve_alias(cmd);
                execute_command(cmd, 1);  // all in background
            }
        } else {
            // Only one command — maybe ends with '&'
            int background = 0;
            char *end = cmd_group + strlen(cmd_group) - 1;
            while (end > cmd_group && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
            if (*end == '&') {
                background = 1;
                *end = '\0';
            }

            char *cmd = cmd_group;
            while (*cmd == ' ' || *cmd == '\t') cmd++;
            if (*cmd == '\0') continue;

            cmd = (char *)resolve_alias(cmd);
            execute_command(cmd, background);
        }
    }
}







