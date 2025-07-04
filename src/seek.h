#ifndef SEEK_H
#define SEEK_H

#define MAX_ARG 100
#define PATH_MAX 4096

void seekCommand(char *args[]);
void search_directory(char *dir, char *target_name, int search_dirs, int search_files, char found_paths[][PATH_MAX], int *found_count);

#endif
