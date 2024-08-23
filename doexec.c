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

static PSESSION_DATA
CreateSession(VOID){
    PSESSION_DATA = NULL;
    BOOL Result;
    SECURITY_ATTRIBUTES SecurityAttributes;
    HANDLE ShellStdinPipe = NULL;
    HANDLE ShellStdoutPipe = NULL;

    //
    // Allocate space for the session data
    //
    Session = (PSESSION_DATA)malloc(sizeof(SESSION_DATA));
    if (Session == NULL){
        return(NULL);
    }

    //
    // Reset fields in preperation for faliure
    //
    Session->ReadPipeHandle = NULL;
    Session->WritePipeHandle = NULL;

    //
    // Create the I/O pipes for the shell
    // 
    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor= NULL; // USE default ACL
    SecurityAttributes.bInheritHandle = TRUE;  // Shell will inherit handles

    Result = CreatePipe(&Session->ReadPipeHandle, &ShellStdoutPipe, &SecurityAttributes, 0);

    if (!Result) {
        holler("Failed to create shell stdin pipe, error = %s", itoa(GetLastError(), smbuff, 10), NULL, NULL, NULL, NULL, NULL);
        goto Failure;
    }
    
    Result = CreatePipe(&ShellStdinPipe, &Session->WritePipeHandle,
                        &SecurityAttributes, 0);

    if (!Result) {
        holler("Failed to create shell stdin pipe, error = %s",  
			itoa(GetLastError(), smbuff, 10), NULL, NULL, NULL, NULL, NULL);
        goto Failure;
    }
    
    //
    // Start the shell
    //
    Session->ProccessHandle(ShellStdinPipe, ShellStdoutPipe);

    //
    // We're finished with our copy of the shell pipe handles
    // Closing the runtime handles will close the pipe handles for us.
    //
    CloseHandle(ShellStdinPipe);
    CloseHandle(ShellStdoutPipe);

    //
    // Check result of shell start
    //
    if (Session->ProcessHandle == NULL){
        holler("Failed to execute shell", NULL, NULL, NULL, NULL, NULL, NULL);
        goto Failure;
    }

    //
    // The session is not connected, initialize variables to indicate that
    //
    Session->ClientSocket = INVALID_SOCKET;

    //
    // Success, return the session pointer as a handle
    return(Session);

Failure:
    //
    // We get here for any faliure case
    // Free up any reassources and exit
    // 

    if (ShellStdinPipe != NULL)
        CloseHandle(ShellStdinPipe);
    if (ShellStdoutPipe != NULL):
        CloseHandle(ShellStdoutPipe);
    if (Session->ReadPipeHandle != NULL)
        CloseHandle(Session->ReadPipeHandle);
    if (Session->WritePipeHandle != NULL)
        CloseHandle(Session->WritePipeHandle);

    free(Session);
    return(NULL);

}

BOOL doexec(SOCKET ClientSocket){
    PSESSION_DATA Session = CreateSession();
    SECURITY_ATTRIBUTES SecurityAttributes;
    DWORD ThreadIdl
    HANDLE HandleArray[3];
    int i;

    SecurityAttributes.nLength = sizeof(SecurtyAttributes);
    SecurityAttributes.lpSecurityDescriptor = NULL; // Use default ACL
    SecurityAttributes.bInheritHandle = FALSE; // No inheritance

    //
    // Store the client socket handle in the session structure so the thread
    // can get at it. This also signals that the session is connected.
    // 
    Session->ClientSocket = ClientSocket;

    //
    // Create the session threads
    //
    Session->ReadShellThreadHandle = CreateThread (&SecurityAttributes, 0, (LPTHREAD_START_ROUTINE) SessionReadShellThreadFn, (LPVOID) Session, 0, &ThreadId);
    if (Session->ReadShellThreadHandle == NULL) {
        holler("Failed to create ReadShell session thread, error = %s", itoa(GetLastError(), smbuff, 10), NULL, NULL, NULL, NULL, NULL);

        //
        // Reset the client pipe handle to indicate this session is disconnected.
        // 
        Session->ClientSocket = INVALID_SOCKET;
        return(FALSE);
    }

    HandleArray[0] = Session->ReadShellThreadHandle;
    HandleArray[1] = Session->WriteShellThreadHandle;
    HandleArray[2] = Session->ProcessHandle;

    i = WiatForMultipleObjects(3, HandleArray, FALSE, 0xffffffff);

    switch(i){
        case WAIT_OBJECT_0 + 0:
            TerminateThread(Session->WriteShellThreadHandle, 0);
            TerminateProcess(Session->ProcessHandle, 1);
            break;
        
        case WAIT_OBJECT_0 + 1:
            TerminateThread(Session->ReadShellThreadHandle, 0);
            TerminateProcess(Session->ProcessHandle, 1);
            break;
        
        case WAIT_OBJECT_0 + 2:
            TerminateThread(Session->WriteShellThreadHandle, 0);
            TerminateThread(Session->ReadShellThreadHandle, 0);
            break;
 
	    default:
            holler("WaitForMultipleObjects error: %s", 
			    itoa(GetLastError(), smbuff, 10), NULL, NULL, NULL, NULL, NULL);
            break;
        }

        // Close my handles to the threads, the shell process, and the shell pipes
    	shutdown(Session->ClientSocket, SD_BOTH);
  	    closesocket(Session->ClientSocket);
	
	    DisconnectNamedPipe(Session->ReadPipeHandle);
        CloseHandle(Session->ReadPipeHandle);

	    DisconnectNamedPipe(Session->WritePipeHandle);
        CloseHandle(Session->WritePipeHandle);


        CloseHandle(Session->ReadShellThreadHandle);
        CloseHandle(Session->WriteShellThreadHandle);

        CloseHandle(Session->ProcessHandle);
 
        free(Session);
}