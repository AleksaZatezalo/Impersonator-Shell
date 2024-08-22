#include <stdlib.h>
#include <winsock2.h>
#include <winbase2.h>

#ifdef GAPING_SECURITY_HOLE // Might Not Be Usefull

#define BUFFER_SIZE 200

extern char *pr00gie;
void holler(char *str, char *p1, char *p2, char *p3, char *p4, char *p5, char *p6);
char smbuff[20];

//
// Structures used to describe each session
//
typedef struct{
    //
    // These fields are filled in at the session creation time.
    //
    HANDLE ReadPipeHandle;
    HANDLE WritePipeHandle;
    HANDLE ProcessHandle;

    //
    //
    // These fields are filled in at the session connect time and are only
    // valid when the session is connection.
    //
    SOCKET ClientSocket;
    HANDLE ReadShellThreadHanle;
    HANDLE WriteShellThreadHandle;

} SESSION_DATA, *PSESSION_DATA

//
// Private Prototypes
//
static HANDLE
StartShell(
    HANDLE StdinPipeHandle,
    HANDLE StdoutPipeHandle
);

static VOID
SessionReadShellThreadFn(
    LPVOID Parameter
);

static VOID
SessionWriteShellThreadFn(
    LPVOID Parameter
);

// **********************************************************************
//
// CreateSession
//
// Creates a new session. Involves creating the shell process and establishing
// pipes for communication with it.
//
// Returns a handle to the session or NULL on failure.
//