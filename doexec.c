/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"

char* doexec(char *command[])
{
    // change argv of doexec after done testing
    char cmd[MAX_CMD_LEN] = "", **p;
    FILE *fp;
    char var[40];
    char *exec;

    strcat(cmd, command[0]);    // change argv of doexec after done testing
    for (p = &command[1]; *p; p++)
    {
        strcat(cmd, " ");
        strcat(cmd, *p);
    }

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

// int main(int argc, char *argv[]){
//     char *exec = doexec(&argv[1]);
//     printf("%s", exec);
//     free(exec);
// }