#include <stdio.h>
#include <string.h>
#include "color.h"

void print_colored(const char *name, const char *type) {
    if (strcmp(type, "dir") == 0) {
        printf("\033[1;34m%s\033[0m\n", name); // Blue for directories
    } else if (strcmp(type, "exe") == 0) {
        printf("\033[1;32m%s\033[0m\n", name); // Green for executables
    } else {
        printf("\033[0m%s\033[0m\n", name); // Default color (white) for regular files
    }
}
