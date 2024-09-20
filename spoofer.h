#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <sddl.h>
#include <stdlib.h>
#include <userenv.h>
#include "doexec.h"
#include "winserver.h"

int username(int sockfd);
char *PrintUserInfoFromToken(DWORD processId);
char *impersonate();