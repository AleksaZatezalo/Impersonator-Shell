/*
* Author: Aleksa Zatezalo
* Date: December 2024
* Description: This C program is meant to recreate winpeas in a C program.
*/


#include "winpeas.h"
#include <aclapi.h>
#include <sddl.h>
#include <userenv.h>
#include <winsvc.h>
#include <iphlpapi.h>
#include <tlhelp32.h>

// Global configuration
static WinPEASConfig g_config;

// Constants definition
const char* SENSITIVE_PATTERNS[] = {
    "password=",
    "pwd=",
    "pass=",
    "admin=",
    "login=",
    // Add more patterns as needed
};

const int NUM_SENSITIVE_PATTERNS = sizeof(SENSITIVE_PATTERNS) / sizeof(SENSITIVE_PATTERNS[0]);

const char* SEARCHABLE_EXTENSIONS[] = {
    ".xml",
    ".txt",
    ".ini",
    ".config",
    ".conf",
    ".cfg",
    ".yml",
    ".yaml"
};

const int NUM_SEARCHABLE_EXTENSIONS = sizeof(SEARCHABLE_EXTENSIONS) / sizeof(SEARCHABLE_EXTENSIONS[0]);

// Banner implementation
void print_banner(void) {
    print_colored("\nWinPEAS - Windows Privilege Escalation Awesome Script\n", FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    print_colored("============================================\n", FOREGROUND_GREEN);
    print_colored("Windows Local Privilege Escalation Scanner\n\n", FOREGROUND_GREEN);
}

// Core system information gathering
void get_system_info(void) {
    SYSTEM_INFO si;
    OSVERSIONINFOEX osvi;
    
    GetSystemInfo(&si);
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    
    printf("\n=== System Information ===\n");
    // Implementation of detailed system info gathering
    // This would include OS version, architecture, patches, etc.
}

// Process checking implementation
void check_running_processes(void) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &pe32)) {
        printf("\n=== Running Processes ===\n");
        do {
            check_process_permissions(pe32.th32ProcessID);
        } while (Process32Next(snapshot, &pe32));
    }

    CloseHandle(snapshot);
}

// Service permission checking
void check_service_permissions(void) {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (scm == NULL) {
        return;
    }

    ENUM_SERVICE_STATUS_PROCESS* services = NULL;
    DWORD bytes_needed = 0;
    DWORD services_returned = 0;
    DWORD resume_handle = 0;

    // First call to get required buffer size
    EnumServicesStatusEx(scm,
        SC_ENUM_PROCESS_INFO,
        SERVICE_WIN32,
        SERVICE_STATE_ALL,
        NULL,
        0,
        &bytes_needed,
        &services_returned,
        &resume_handle,
        NULL);

    if (bytes_needed > 0) {
        services = (ENUM_SERVICE_STATUS_PROCESS*)malloc(bytes_needed);
        if (services != NULL) {
            if (EnumServicesStatusEx(scm,
                SC_ENUM_PROCESS_INFO,
                SERVICE_WIN32,
                SERVICE_STATE_ALL,
                (LPBYTE)services,
                bytes_needed,
                &bytes_needed,
                &services_returned,
                &resume_handle,
                NULL)) {
                
                printf("\n=== Service Permissions ===\n");
                for (DWORD i = 0; i < services_returned; i++) {
                    check_service_security(scm, services[i].lpServiceName);
                }
            }
            free(services);
        }
    }

    CloseServiceHandle(scm);
}

// Registry permission checking
void check_registry_permissions(void) {
    printf("\n=== Registry Permissions ===\n");
    
    const char* important_keys[] = {
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce",
        "SYSTEM\\CurrentControlSet\\Services",
        // Add more important registry keys
    };

    for (int i = 0; i < sizeof(important_keys)/sizeof(important_keys[0]); i++) {
        check_registry_key_permissions(HKEY_LOCAL_MACHINE, important_keys[i]);
        check_registry_key_permissions(HKEY_CURRENT_USER, important_keys[i]);
    }
}

// Network information gathering
void enumerate_network_info(void) {
    printf("\n=== Network Information ===\n");
    
    // Get adapter information
    PIP_ADAPTER_INFO adapter_info = NULL;
    ULONG out_buf_len = 0;
    
    if (GetAdaptersInfo(NULL, &out_buf_len) == ERROR_BUFFER_OVERFLOW) {
        adapter_info = (IP_ADAPTER_INFO*)malloc(out_buf_len);
        if (adapter_info != NULL) {
            if (GetAdaptersInfo(adapter_info, &out_buf_len) == NO_ERROR) {
                PIP_ADAPTER_INFO p_adapter = adapter_info;
                while (p_adapter) {
                    printf("Adapter Name: %s\n", p_adapter->Description);
                    printf("IP Address: %s\n", p_adapter->IpAddressList.IpAddress.String);
                    p_adapter = p_adapter->Next;
                }
            }
            free(adapter_info);
        }
    }
    
    // Additional network enumeration code would go here
}

// Main function
int main(int argc, char* argv[]) {
    print_banner();
    
    // Parse command line arguments and initialize configuration
    init_config(argc, argv);
    
    // Start information gathering
    get_system_info();
    check_running_processes();
    check_service_permissions();
    check_registry_permissions();
    enumerate_network_info();
    check_user_privileges();
    search_sensitive_files("C:\\");
    
    // Additional checks based on configuration
    if (g_config.full_check) {
        // Perform more intensive checks
        check_stored_credentials();
        check_powershell_history();
        // Add more intensive checks
    }
    
    return 0;
}

// Configuration initialization
WinPEASConfig* init_config(void) {
    WinPEASConfig* config = (WinPEASConfig*)malloc(sizeof(WinPEASConfig));
    if (config != NULL) {
        config->timestamp_enabled = false;
        config->full_check = false;
        config->excel_check = false;
    }
    return config;
}

// Utility function implementations
void print_colored(const char* text, int color) {
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO console_info;
    WORD original_attributes;

    GetConsoleScreenBufferInfo(h_console, &console_info);
    original_attributes = console_info.wAttributes;

    SetConsoleTextAttribute(h_console, color);
    printf("%s", text);
    SetConsoleTextAttribute(h_console, original_attributes);
}