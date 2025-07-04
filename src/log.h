#ifndef LOG_H
#define LOG_H

void add_to_log(char *command);
void load_log();
void print_log();
void purge_log();
void execute_log_command(int index);

// Forward declaration to avoid circular dependency with commandhandler.h
void execute_command(char *cmd,int background);

#endif
