/*
* Author: Aleksa Zatezalo
* Date: September 2023
* Description: The impersonator shell that combines netpipe.c, doexec.c, and spoofer.c into a shell. 
*/

#include "doexec.h"
#include "netpipe.h"

/*
* TO DO
* 1. Compile on Windows VM & Test
* 2. Automatic `whoami /priv`
* 3. Implements Priv Esc attack If Priv
*/

int main(int argc, char **argv)
{
    int c, protocol = SOCK_STREAM, listen = 0, family = AF_UNSPEC;

    opterr = 0;

    while ((c = getopt(argc, argv, "lu46")) != -1)
    {
        switch (c)
        {
        case 'l':
            listen = 1;
            break;
        case 'u':
            protocol = SOCK_DGRAM;
            break;
        case '4':
            family = AF_INET;
            break;
        case '6':
            family = AF_INET6;
            break;
        }
    }

    if (listen == 0)
    {
        if (optind + 2 != argc)
        {
            Help(argv[0]);
        }

        Client(argv[optind], argv[optind + 1], &protocol, &family);
    }
    else
    {
        char *hostname = NULL, *port;

        if (optind + 1 == argc)
        {
            port = argv[optind];
        }
        else if (optind + 2 == argc)
        {
            hostname = argv[optind];
            port = argv[optind + 1];
        }
        else
        {
            Help(argv[0]);
        }

        Server(hostname, port, &protocol, &family);
    }

    return 0;
}