#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "seek.h"

void search_directory(char *dir, char *target_name, int search_dirs, int search_files, char found_paths[][PATH_MAX], int *found_count)
{
    DIR *d = opendir(dir);
    if (!d)
        return;

    struct dirent *entry;
    struct stat st;
    while ((entry = readdir(d)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        if (stat(path, &st) == -1)
            continue;

        if (search_dirs && S_ISDIR(st.st_mode) && strcmp(entry->d_name, target_name) == 0)
        {
            strcpy(found_paths[(*found_count)++], path);
        }
        if (search_files && S_ISREG(st.st_mode) && strcmp(entry->d_name, target_name) == 0)
        {
            strcpy(found_paths[(*found_count)++], path);
        }

        if (S_ISDIR(st.st_mode))
        {
            search_directory(path, target_name, search_dirs, search_files, found_paths, found_count);
        }
    }
    closedir(d);
}

void seekCommand(char *args[])
{
    int search_dirs = 1, search_files = 1, execute_flag = 0;
    char *target_name = NULL;
    char *target_dir = ".";
    struct stat st;
    int found_count = 0;
    char found_paths[100][PATH_MAX];
    // Parse flags and arguments
    for (int i = 1; args[i] != NULL; i++)
    {
        if (args[i][0] == '-')
        {
            if (strchr(args[i], 'd'))
                search_files = 0;
            if (strchr(args[i], 'f'))
                search_dirs = 0;
            if (strchr(args[i], 'e'))
                execute_flag = 1;
        }
        else if (target_name == NULL)
        {
            target_name = args[i];
        }
        else
        {
            target_dir = args[i];
        }
    }
    // Handle invalid flag combinations
    if (!search_dirs && !search_files)
    {
        printf("Invalid flags!\n");
        return;
    }
    // Start searching from the target directory
    search_directory(target_dir, target_name, search_dirs, search_files, found_paths, &found_count);
    if (found_count == 0)
    {
        printf("No match found!\n");
        return;
    }

    if (execute_flag && found_count == 1)
    {
        if (stat(found_paths[0], &st) == 0)
        {
            if (S_ISREG(st.st_mode))
            {
                // Print file content
                FILE *file = fopen(found_paths[0], "r");
                if (file)
                {
                    char line[256];
                    while (fgets(line, sizeof(line), file))
                    {
                        printf("%s", line);
                    }
                    fclose(file);
                }
                else
                {
                    printf("Missing permissions for task!\n");
                }
            }
            else if (S_ISDIR(st.st_mode))
            {
                char cwd[PATH_MAX];
                char *prev_pwd = getenv("PWD");

                if (chdir(found_paths[0]) == 0)
                {
                    // Update OLDPWD and PWD
                    if (prev_pwd)
                        setenv("OLDPWD", prev_pwd, 1);

                    if (getcwd(cwd, sizeof(cwd)) != NULL)
                    {
                        setenv("PWD", cwd, 1);
                        printf("Changed directory to %s\n", cwd);
                    }
                    else
                    {
                        perror("getcwd");
                    }
                }
                else
                {
                    printf("Missing permissions for task!\n");
                }
            }
        }
    }
    else
    {
        // Print all found paths
        for (int i = 0; i < found_count; i++)
        {
            printf("%s\n", found_paths[i]);
        }
    }
}
