/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"

char* doexec(char *command)
{
    // change argv of doexec after done testing
    char cmd[MAX_CMD_LEN] = "", **p;
    FILE *fp;
    char var[40];
    char *exec;

    strcat(cmd, command); 
    // Storage for output
    exec = malloc(sizeof(char) * 10);
    memset(exec, 0, 10);

    fp = popen(cmd, "r");
    if (exec == NULL){
        printf("Memory not allocated.\n");
        exit(0);
    }


    while (fgets(var, sizeof(var), fp) != NULL) 
    {
        int size = (strlen(exec)+ strlen(var)) * sizeof(char);
        exec = realloc(exec, size );
        strcat(exec, var); 
    }
    pclose(fp);
    return exec;
}