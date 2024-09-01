/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Doexec is a C program that executes provided commands in terminal and returns output. 
*/

#include "doexec.h"

void doexec(char *argv[])
{
    // change argv of doexec after done testing
    char cmd[MAX_CMD_LEN] = "", **p;
    FILE *fp,*outputfile;
    char var[40];

    strcat(cmd, argv[1]);    // change argv of doexec after done testing
    for (p = &argv[2]; *p; p++)
    {
        strcat(cmd, " ");
        strcat(cmd, *p);
    }

    fp = popen(cmd, "r");
    printf("COMMAND: %s \n", cmd);
    while (fgets(var, sizeof(var), fp) != NULL) 
    {
        printf("OUTPUT: %s", var);
        // Do not print
        // Create dynamically longer array
        // Somehow plug it into socket functionality
        // Have it function like netcat
    }
    pclose(fp);

}

// Remove main func after you are done testing
// Change argv of doexec after removing main
int main(int argc, char *argv[]){
    printf("WELCOME TO DOEXEC \n");
    doexec(argv);
    return 0;
}