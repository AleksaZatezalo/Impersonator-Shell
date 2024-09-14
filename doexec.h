#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MAX_CMD_LEN 1000

char* doexec(char *command);
char* norm_exec(char *command);