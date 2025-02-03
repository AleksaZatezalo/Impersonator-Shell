/*
* Author: Aleksa Zatezalo
* Date: Februrary 2024
* Description: LSASS is a C program that dumps password hashes from LSASSs. 
*/

#include <stdio.h>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "DbgHelp.lib")