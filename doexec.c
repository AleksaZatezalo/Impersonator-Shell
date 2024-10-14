/*
* Author: Aleksa Zatezalo
* Date: September 2024
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"

char* doExec(char *command, int user_input)
{
    FILE *pPipe;
    char var[128];
    char *exec;

    if (user_input) {
        char *err = " 2>&1";
        command[strlen(command) - 1] = '\0';
        strcat(command, err);
    }

    if ((pPipe = _popen(command, "rt")) == NULL)
    {
        exit(1);
    }

    exec = malloc(sizeof(char) * 10);
    memset(exec, 0, 10);

    if (exec == NULL){
        printf("Memory not allocated.\n");
        exit(0);
    }

    int size = 0;
    while (fgets(var, 128, pPipe) != NULL) 
    {
        size = size + strlen(var) *sizeof(char) + 1;
        exec = realloc(exec, size);
        strcat(exec, var); 
    }
    feof(pPipe);
    _pclose(pPipe);
    return exec;
}