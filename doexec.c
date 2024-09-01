/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"
#include <stdlib.h>
#include <string.h>

void doexec(char *argv[])
{
    char cmd[MAX_CMD_LEN] = "", **p;
    FILE *fp,*outputfile;
    char var[40];

    strcat(cmd, argv[1]);
    for (p = &argv[2]; *p; p++)
    {
        strcat(cmd, " ");
        strcat(cmd, *p);
    }

    fp = popen(cmd, "r");
    while (fgets(var, sizeof(var), fp) != NULL) 
    {
        printf("%s", var);
    }
    pclose(fp);

}

int main(int argc, char *argv[])
{
    doexec(argv);
}