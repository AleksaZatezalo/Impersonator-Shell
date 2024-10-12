#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include <sddl.h>
#include <stdlib.h>
#include <userenv.h>
#include "doexec.h"
#include "winserver.h"

char *PrintUserInfoFromToken(DWORD processId);
char *EnablePrivileges(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege);
char *Impersonate(int pid);
char *EnableDebugPrivilege();