#include <netdb.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXBUF 65536
#define BACKLOG 1

void *Thread(void *arg);
int CreateClientSocket(char *address, char *port, int *type, int *family);
void Client(char *address, char *port, int *type, int *family);
int CreateServerSocket(char *address, char *port, int *type, int *family);
void Server(char *address, char *port, int *type, int *family);
void PrintError(char *function);
void Help(char *name);