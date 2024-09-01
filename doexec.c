/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LEN 100

int doexec(int argc, char *argv[])
{
    char cmd[MAX_CMD_LEN] = "", **p;

    if (argc < 2) /*No command specified.*/
    {
        fprintf(stderr, "Usage: ./program_name terminal_command");
    }

    else
    {
        strcat(cmd, argv[1]);
        for (p = &argv[2]; *p; p++)
        {
            strcat(cmd, " ");
            strcat(cmd, *p);
        }
        system(cmd);
    }

    return 0;
}
