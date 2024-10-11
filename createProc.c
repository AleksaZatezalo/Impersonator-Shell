/*
* Author: Aleksa Zatezalo
* Date: September 2024
* Description: createProc is a C program that spawns a proccess. 
*/
#include <windows.h>
#include <stdio.h>
#include <tchar.h>

char *createProc(char *procName){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    
    // Start the child process.
    if (!CreateProcess(NULL, // No module name (use command line)
        procName, // Command line
        NULL, // Process handle not inheritable
        NULL, // Thread handle not inheritable
        FALSE, // Set handle inheritance to FALSE
        0, // No creation flags
        NULL, // Use parent’s environment block
        NULL, // Use parent’s starting directory
        &si, // Pointer to STARTUPINFO structure
        &pi) // Pointer to PROCESS_INFORMATION structure
        ){
            printf("[-] Create Proccess Failed");
            return;
        }
        
        printf("PID: %d \r\n", pi.dwProcessId);
        // Wait until child process exits.
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD code;
        GetExitCodeProcess(pi.hProcess, &code);
        // Close process and thread handles.
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
}
