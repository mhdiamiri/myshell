#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define BUFFER_LEN 511

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void shell_main_loop();
char* read_line();
char** split_lines(char *line);
int execute_command(char** args, int in);
int lunch(char** args, bool bg);
void kill_child();
int shell_cd(char **args);

