#include <windows.h>
#include <stdio.h>
#include <tchar.h>

void DumpProcessToken(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess == NULL) {
        printf("Error opening process: %d\n", GetLastError());
        return;
    }

    HANDLE hToken;
    if (!OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        printf("Error opening process token: %d\n", GetLastError());
        CloseHandle(hProcess);
        return;
    }

    DWORD tokenInfoLength = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenInfoLength);
    PTOKEN_USER pTokenUser = (PTOKEN_USER)malloc(tokenInfoLength);
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, tokenInfoLength, &tokenInfoLength)) {
        printf("Error getting token information: %d\n", GetLastError());
        free(pTokenUser);
        CloseHandle(hToken);
        CloseHandle(hProcess);
        return;
    }

    // Display the user SID
    TCHAR sidString[256];
    if (ConvertSidToStringSid(pTokenUser->User.Sid, &sidString)) {
        printf("User SID: %s\n", sidString);
        LocalFree(sidString);
    } else {
        printf("Error converting SID to string: %d\n", GetLastError());
    }

    // Cleanup
    free(pTokenUser);
    CloseHandle(hToken);
    CloseHandle(hProcess);
}

int main() {
    DWORD processId;

    printf("Enter the process ID: ");
    scanf("%lu", &processId);

    DumpProcessToken(processId);

    return 0;
}
