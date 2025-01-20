#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_PORT 65535
#define MIN_PORT 1
#define DEFAULT_TIMEOUT 1000 // milliseconds

typedef struct {
    char* target;
    int start_port;
    int end_port;
    int timeout;
    int show_closed;
} ScanOptions;

// Function to print usage
void print_usage(char* prog_name) {
    printf("Usage: %s <options>\n", prog_name);
    printf("Options:\n");
    printf("  -t <target>     Target IP address\n");
    printf("  -p <port-range> Port range (e.g., 80 or 1-1000)\n");
    printf("  -T <timeout>    Timeout in milliseconds (default: 1000)\n");
    printf("  -a              Show closed ports (default: only open)\n");
    printf("\nExample: %s -t 192.168.1.1 -p 1-100\n", prog_name);
}

// Function to parse command line arguments
ScanOptions parse_args(int argc, char* argv[]) {
    ScanOptions options = {
        .target = NULL,
        .start_port = 1,
        .end_port = 1000,
        .timeout = DEFAULT_TIMEOUT,
        .show_closed = 0
    };

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            options.target = argv[++i];
        }
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            char* port_range = argv[++i];
            char* hyphen = strchr(port_range, '-');
            if (hyphen) {
                *hyphen = '\0';
                options.start_port = atoi(port_range);
                options.end_port = atoi(hyphen + 1);
            }
            else {
                options.start_port = options.end_port = atoi(port_range);
            }
        }
        else if (strcmp(argv[i], "-T") == 0 && i + 1 < argc) {
            options.timeout = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "-a") == 0) {
            options.show_closed = 1;
        }
    }

    // Validate port range
    if (options.start_port < MIN_PORT) options.start_port = MIN_PORT;
    if (options.end_port > MAX_PORT) options.end_port = MAX_PORT;
    if (options.start_port > options.end_port) {
        int temp = options.start_port;
        options.start_port = options.end_port;
        options.end_port = temp;
    }

    return options;
}

// Function to attempt connection to a port
int try_connect(const char* target, int port, int timeout) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return -1;
    }

    // Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);

    // Set up address structure
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, target, &(addr.sin_addr));

    // Attempt connection
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    // Wait for connection or timeout
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(sock, &write_fds);

    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    int result = select(0, NULL, &write_fds, NULL, &tv);

    // Clean up
    closesocket(sock);

    return result > 0;
}

// Function to perform service identification
const char* identify_service(int port) {
    // Basic service identification based on common ports
    switch (port) {
        case 21: return "FTP";
        case 22: return "SSH";
        case 23: return "Telnet";
        case 25: return "SMTP";
        case 53: return "DNS";
        case 80: return "HTTP";
        case 110: return "POP3";
        case 143: return "IMAP";
        case 443: return "HTTPS";
        case 445: return "SMB";
        case 3306: return "MySQL";
        case 3389: return "RDP";
        case 5432: return "PostgreSQL";
        case 8080: return "HTTP-ALT";
        default: return "Unknown";
    }
}

int main(int argc, char* argv[]) {
    // Check arguments
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    // Parse command line arguments
    ScanOptions options = parse_args(argc, argv);
    if (!options.target) {
        printf("Error: Target IP address is required\n");
        return 1;
    }

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return 1;
    }

    // Print scan information
    printf("\nStarting basic port scan...\n");
    printf("Target: %s\n", options.target);
    printf("Port range: %d-%d\n", options.start_port, options.end_port);
    printf("Timeout: %dms\n\n", options.timeout);

    // Record start time
    time_t start_time = time(NULL);

    // Perform scan
    int open_ports = 0;
    int closed_ports = 0;

    printf("PORT\tSTATE\tSERVICE\n");
    printf("----\t-----\t-------\n");

    for (int port = options.start_port; port <= options.end_port; port++) {
        int is_open = try_connect(options.target, port, options.timeout);
        
        if (is_open) {
            printf("%d\topen\t%s\n", port, identify_service(port));
            open_ports++;
        }
        else if (options.show_closed) {
            printf("%d\tclosed\t%s\n", port, identify_service(port));
            closed_ports++;
        }
    }

    // Print summary
    time_t end_time = time(NULL);
    printf("\nScan completed in %ld seconds\n", end_time - start_time);
    printf("Open ports: %d\n", open_ports);
}