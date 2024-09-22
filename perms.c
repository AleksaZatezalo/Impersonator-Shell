#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <sddl.h>

void PrintTokenPermissions(DWORD pid) {
    HANDLE hProcess = NULL;
    HANDLE hToken = NULL;
    PTOKEN_PRIVILEGES pTokenPrivileges = NULL;
    DWORD dwSize = 0;

    // Open the process with the required permissions
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (hProcess == NULL) {
        printf("OpenProcess failed: %u\n", GetLastError());
        return;
    }

    // Open the token associated with the process
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        printf("OpenProcessToken failed: %u\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    // Get the size needed for the token privileges
    if (!GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwSize) && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        printf("GetTokenInformation failed: %u\n", GetLastError());
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return;
    }

    // Allocate memory for the token privileges
    pTokenPrivileges = (PTOKEN_PRIVILEGES)malloc(dwSize);
    if (pTokenPrivileges == NULL) {
        printf("Memory allocation failed\n");
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return;
    }

    // Retrieve the token privileges
    if (!GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, dwSize, &dwSize)) {
        printf("GetTokenInformation failed: %u\n", GetLastError());
        free(pTokenPrivileges);
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return;
    }

    // Print the privileges
    printf("Privileges for PID %u:\n", pid);
    for (DWORD i = 0; i < pTokenPrivileges->PrivilegeCount; i++) {
        LPTSTR privilegeName;
        DWORD nameSize = 0;

        // Get the privilege name size
        LookupPrivilegeName(NULL, &pTokenPrivileges->Privileges[i].Luid, NULL, &nameSize);
        privilegeName = (LPTSTR)malloc(nameSize * sizeof(TCHAR));
        if (LookupPrivilegeName(NULL, &pTokenPrivileges->Privileges[i].Luid, privilegeName, &nameSize)) {
            printf("- %s\n", privilegeName);
        } else {
            printf("Failed to lookup privilege name: %u\n", GetLastError());
        }
        free(privilegeName);
    }

    // Clean up
    free(pTokenPrivileges);
    CloseHandle(hToken);
    CloseHandle(hProcess);
}

int main() {
    DWORD pid;
    printf("Enter the PID of the process: ");
    scanf("%u", &pid);
    PrintTokenPermissions(pid);
    return 0;
}