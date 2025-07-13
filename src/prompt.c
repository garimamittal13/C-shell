#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include "commandhandler.h"
#include "color.h" // Include the color header

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif


void displayPrompt()
{
    char *username = getenv("USER");
    if (!username)
    {
        username = "unknown";
    }

    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }

    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd");
        exit(EXIT_FAILURE);
    }

    char *home_dir = getenv("HOME");
    printf("\033[1;36m%s@%s:\033[0m", username, hostname); // Cyan color for username@hostname

    if (home_dir != NULL && strncmp(cwd, home_dir, strlen(home_dir)) == 0)
    {
        char *relative_path = cwd + strlen(home_dir);
        printf("\033[1;34m~%s\033[0m$ ", relative_path); // Blue for the relative path
    }
    else
    {
        printf("\033[1;34m%s\033[0m$ ", cwd); // Blue for the full path
    }
}
