#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "inputhandler.h"
#include "commandhandler.h"
#include "myshrc.h"

#define MAX_INPUT 1024
#define MAX_ARG 100

// Function to determine if the input contains pipes or redirection
int contains_special_symbols(char *cmd) {
    return strchr(cmd, '|') || strchr(cmd, '<') || strchr(cmd, '>'); 
}

void handle_input(char *input) {
    char *cmds[MAX_ARG];
    int cmd_count = 0;

    char *token = strtok(input, ";");
    while (token != NULL) {
        cmds[cmd_count++] = token;
        token = strtok(NULL, ";");
    }

    for (int i = 0; i < cmd_count; i++) {
        char *cmd = cmds[i];
        while (*cmd == ' ' || *cmd == '\t') cmd++;  // Remove leading spaces

        // Resolve alias before execution
        cmd = (char *)resolve_alias(cmd); //how does it work?

        int background = 0;
        char *bg_position = strchr(cmd, '&');
        if (bg_position != NULL) {
            *bg_position = '\0';  // Remove the '&' from the command string
            background = 1;
        }

        // Remove trailing spaces after handling '&'
        char *end = cmd + strlen(cmd) - 1;
        while (end > cmd && (*end == ' ' || *end == '\t')) {
            *end = '\0';
            end--;
        }

        // Tokenize cmd into args[]
        char *args[MAX_ARG];
        int arg_count = 0;
        char *cmd_copy = strdup(cmd); // Make a copy of cmd, as strtok modifies it
        char *arg_token = strtok(cmd_copy, " \t\n");
        while (arg_token != NULL && arg_count < MAX_ARG - 1) {
            args[arg_count++] = arg_token;
            arg_token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL; // Null-terminate the argument list

        // Check for fg and bg commands
        if (strncmp(cmd, "fg", 2) == 0) {
            fgCommand(args);
        } else if (strncmp(cmd, "bg", 2) == 0) {
            bgCommand(args);
        } else if (contains_special_symbols(cmd)) {
            execute_command_with_redirection(cmd);
        } else if (*cmd != '\0') {
            execute_command(cmd, background);
        }

        free(cmd_copy); // Free the duplicated command string
    }
}


