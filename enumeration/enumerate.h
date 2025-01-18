/*
* Author: Aleksa Zatezalo
* Date: December 2024
* Description: This C program is meant to recreate winpeas in a C program.
*/


#ifndef WINPEAS_H
#define WINPEAS_H

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <stdbool.h>

// Configuration flags
typedef struct {
    bool timestamp_enabled;
    bool full_check;
    bool excel_check;
} WinPEASConfig;

// Function to check registry permissions
typedef struct {
    DWORD access_mask;
    char* account_name;
} ACLEntry;

// Core system information functions
void get_system_info(void);
void check_hotfixes(void);
void check_antivirus(void);

// Registry checking functions
void check_registry_permissions(void);
void check_lsa_settings(void);
void check_stored_credentials(void);
void check_winlogon_credentials(void);
void check_rdp_settings(void);

// Service and process functions
void check_running_processes(void);
void check_service_permissions(void);
void check_unquoted_service_paths(void);
void check_startup_applications(void);

// Network information functions
void enumerate_network_info(void);
void check_wifi_credentials(void);
void check_network_shares(void);

// User information functions
void enumerate_users(void);
void check_user_privileges(void);
void check_token_privileges(void);
void check_stored_credentials(void);

// File system functions
void search_sensitive_files(const char* search_path);
void check_powershell_history(void);
void check_registry_for_passwords(void);

// Utility functions
void print_banner(void);
void print_colored(const char* text, int color);
bool check_file_permissions(const char* filepath);
void scan_directory_recursive(const char* dir_path, void (*callback)(const char*));

// Registry helper functions
BOOL is_registry_accessible(HKEY hKey, const char* subkey);
void enumerate_registry_keys(HKEY hKey, const char* subkey);
void search_registry_for_pattern(HKEY hKey, const char* pattern);

// Network helper functions
void get_network_adapters(void);
void get_network_shares(void);
void get_network_routes(void);

// Configuration functions
WinPEASConfig* init_config(void);
void set_config_option(WinPEASConfig* config, const char* option);
void cleanup_config(WinPEASConfig* config);

// Constants for sensitive patterns
extern const char* SENSITIVE_PATTERNS[];
extern const int NUM_SENSITIVE_PATTERNS;

// Constants for file extensions to search
extern const char* SEARCHABLE_EXTENSIONS[];
extern const int NUM_SEARCHABLE_EXTENSIONS;

#endif // WINPEAS_H
