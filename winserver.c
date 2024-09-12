#include <winsock2.h>
#include <Windows.h>
#include <stdio.h>

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
        recv(client, buffer, sizeof(buffer), 0);
        printf("            Client says: %s \n",buffer);
        closesocket(client);
        printf("Client disconnected.\n");
    }
}