#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define DEFAULT_PORT 21

typedef struct {
    SOCKET control_socket;
    SOCKET data_socket;
    char buffer[BUFFER_SIZE];
} FTPClient;

// Initialize the FTP client
FTPClient* ftp_init() {
    FTPClient* client = (FTPClient*)malloc(sizeof(FTPClient));
    if (!client) return NULL;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        free(client);
        return NULL;
    }

    client->control_socket = INVALID_SOCKET;
    client->data_socket = INVALID_SOCKET;
    return client;
}

// Connect to FTP server
int ftp_connect(FTPClient* client, const char* host, int port) {
    struct sockaddr_in server;
    
    // Create socket
    client->control_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client->control_socket == INVALID_SOCKET) {
        return 0;
    }

    // Setup server address
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    inet_pton(AF_INET, host, &(server.sin_addr));

    // Connect to server
    if (connect(client->control_socket, (struct sockaddr*)&server, sizeof(server)) != 0) {
        closesocket(client->control_socket);
        return 0;
    }

    // Read welcome message
    recv(client->control_socket, client->buffer, BUFFER_SIZE, 0);
    printf("Server: %s", client->buffer);

    return 1;
}

// Send command to server
int ftp_send_command(FTPClient* client, const char* command) {
    printf("Sending: %s", command);
    if (send(client->control_socket, command, strlen(command), 0) < 0) {
        return 0;
    }

    // Get response
    memset(client->buffer, 0, BUFFER_SIZE);
    recv(client->control_socket, client->buffer, BUFFER_SIZE, 0);
    printf("Server: %s", client->buffer);

    return 1;
}

// Login to FTP server
int ftp_login(FTPClient* client, const char* username, const char* password) {
    char command[BUFFER_SIZE];

    // Send username
    snprintf(command, sizeof(command), "USER %s\r\n", username);
    if (!ftp_send_command(client, command)) {
        return 0;
    }

    // Send password
    snprintf(command, sizeof(command), "PASS %s\r\n", password);
    if (!ftp_send_command(client, command)) {
        return 0;
    }

    return strstr(client->buffer, "230") != NULL; // 230 means login successful
}

// Enter passive mode
int ftp_passive_mode(FTPClient* client, int* data_port) {
    // Send PASV command
    if (!ftp_send_command(client, "PASV\r\n")) {
        return 0;
    }

    // Parse response for IP and port
    char* start = strchr(client->buffer, '(');
    char* end = strchr(client->buffer, ')');
    if (!start || !end) return 0;

    start++;
    *end = '\0';

    int ip[4], port[2];
    sscanf(start, "%d,%d,%d,%d,%d,%d", 
           &ip[0], &ip[1], &ip[2], &ip[3], &port[0], &port[1]);

    *data_port = port[0] * 256 + port[1];
    return 1;
}

// Download file from server
int ftp_download(FTPClient* client, const char* remote_file, const char* local_file) {
    int data_port;
    if (!ftp_passive_mode(client, &data_port)) {
        return 0;
    }

    // Open data connection
    struct sockaddr_in data_addr;
    client->data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    memcpy(&data_addr.sin_addr, &((struct sockaddr_in*)(getsockname(client->control_socket, NULL, NULL)))->sin_addr, sizeof(struct in_addr));

    if (connect(client->data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) != 0) {
        return 0;
    }

    // Send RETR command
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "RETR %s\r\n", remote_file);
    if (!ftp_send_command(client, command)) {
        closesocket(client->data_socket);
        return 0;
    }

    // Open local file
    FILE* file = fopen(local_file, "wb");
    if (!file) {
        closesocket(client->data_socket);
        return 0;
    }

    // Receive and write data
    int bytes;
    while ((bytes = recv(client->data_socket, client->buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(client->buffer, 1, bytes, file);
    }

    fclose(file);
    closesocket(client->data_socket);
    return 1;
}

// Upload file to server
int ftp_upload(FTPClient* client, const char* local_file, const char* remote_file) {
    int data_port;
    if (!ftp_passive_mode(client, &data_port)) {
        return 0;
    }

    // Open data connection
    struct sockaddr_in data_addr;
    client->data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    memcpy(&data_addr.sin_addr, &((struct sockaddr_in*)(getsockname(client->control_socket, NULL, NULL)))->sin_addr, sizeof(struct in_addr));

    if (connect(client->data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) != 0) {
        return 0;
    }

    // Send STOR command
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "STOR %s\r\n", remote_file);
    if (!ftp_send_command(client, command)) {
        closesocket(client->data_socket);
        return 0;
    }

    // Open local file
    FILE* file = fopen(local_file, "rb");
    if (!file) {
        closesocket(client->data_socket);
        return 0;
    }

    // Read and send data
    int bytes;
    while ((bytes = fread(client->buffer, 1, BUFFER_SIZE, file)) > 0) {
        send(client->data_socket, client->buffer, bytes, 0);
    }

    fclose(file);
    closesocket(client->data_socket);
    return 1;
}

// List directory contents
int ftp_list(FTPClient* client) {
    int data_port;
    if (!ftp_passive_mode(client, &data_port)) {
        return 0;
    }

    // Open data connection
    struct sockaddr_in data_addr;
    client->data_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    data_addr.sin_family = AF_INET;
    data_addr.sin_port = htons(data_port);
    memcpy(&data_addr.sin_addr, &((struct sockaddr_in*)(getsockname(client->control_socket, NULL, NULL)))->sin_addr, sizeof(struct in_addr));

    if (connect(client->data_socket, (struct sockaddr*)&data_addr, sizeof(data_addr)) != 0) {
        return 0;
    }

    // Send LIST command
    if (!ftp_send_command(client, "LIST\r\n")) {
        closesocket(client->data_socket);
        return 0;
    }

    // Receive and print data
    int bytes;
    while ((bytes = recv(client->data_socket, client->buffer, BUFFER_SIZE, 0)) > 0) {
        client->buffer[bytes] = '\0';
        printf("%s", client->buffer);
    }

    closesocket(client->data_socket);
    return 1;
}

// Cleanup and close connection
void ftp_quit(FTPClient* client) {
    if (client) {
        ftp_send_command(client, "QUIT\r\n");
        closesocket(client->control_socket);
        WSACleanup();
        free(client);
    }
}

// Example usage
int main() {
    FTPClient* client = ftp_init();
    if (!client) {
        printf("Failed to initialize FTP client\n");
        return 1;
    }

    // Connect to server
    if (!ftp_connect(client, "127.0.0.1", DEFAULT_PORT)) {
        printf("Failed to connect to server\n");
        ftp_quit(client);
        return 1;
    }

    // Login
    if (!ftp_login(client, "username", "password")) {
        printf("Failed to login\n");
        ftp_quit(client);
        return 1;
    }

    // List directory contents
    printf("\nDirectory listing:\n");
    ftp_list(client);

    // Download a file
    printf("\nDownloading file...\n");
    if (ftp_download(client, "remote.txt", "local.txt")) {
        printf("Download successful\n");
    } else {
        printf("Download failed\n");
    }

    // Upload a file
    printf("\nUploading file...\n");
    if (ftp_upload(client, "local.txt", "remote_upload.txt")) {
        printf("Upload successful\n");
    } else {
        printf("Upload failed\n");
    }

    // Cleanup
    ftp_quit(client);
    return 0;
}