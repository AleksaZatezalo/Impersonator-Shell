#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include "doexec.h"

int header(int sockfd);
int command_prompt(int sockfd);
void Help(char *name);
int server(int port);
int client(char *rhost, int port);