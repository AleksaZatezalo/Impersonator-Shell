#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <sddl.h>
#include <stdlib.h>
#include <userenv.h>
#include "doexec.h"
#include "winserver.h"

char *printUserInfoFromToken(DWORD processId);
char *impersonate(int pid);
char *enableDebugPrivilege();