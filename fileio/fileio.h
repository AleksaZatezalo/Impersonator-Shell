#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <windows.h>
#include <winhttp.h>
#include <stdio.h>

// Status codes for download operation
typedef enum {
    DOWNLOAD_SUCCESS = 0,
    DOWNLOAD_CONNECT_FAILED,
    DOWNLOAD_REQUEST_FAILED,
    DOWNLOAD_WRITE_FAILED,
    DOWNLOAD_MEMORY_ERROR,
    DOWNLOAD_INVALID_ARGS
} DownloadStatus;

// Structure to hold download options
typedef struct {
    const wchar_t* url;         // URL to download from
    const wchar_t* filename;    // Local filename to save as
    BOOL show_progress;         // Whether to show download progress
    DWORD timeout;             // Connection timeout in milliseconds
} DownloadOptions;

// Main download function
DownloadStatus download_file(const DownloadOptions* options);

// Helper function to parse URLs
BOOL parse_url(const wchar_t* url, wchar_t* host, size_t host_len, 
               wchar_t* path, size_t path_len, WORD* port);

// Get a human-readable error message
const char* get_download_error(DownloadStatus status);

#endif // DOWNLOADER_H