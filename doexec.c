/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"

char* doexec(char *argv[])
{
    // change argv of doexec after done testing
    char cmd[MAX_CMD_LEN] = "", **p;
    FILE *fp;
    char var[40];
    char *exec;

    strcat(cmd, argv[1]);    // change argv of doexec after done testing
    for (p = &argv[2]; *p; p++)
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
        int size = sizeof(exec)+ sizeof(var);
        exec = realloc(exec, size + 41);
        strcat(exec, var); 
    }
    pclose(fp);
    return exec;

}

// Remove main func after you are done testing
// Change argv of doexec after removing main
int main(int argc, char *argv[]){
    char *command = doexec(argv);
    printf(command);
    return 0;
}