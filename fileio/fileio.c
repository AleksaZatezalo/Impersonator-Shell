#include "fileio.h"
#include <stdlib.h>

#pragma comment(lib, "winhttp.lib")

#define DEFAULT_BUFFER_SIZE 8192
#define MAX_HOST_LENGTH 256
#define MAX_PATH_LENGTH 2048

static void show_download_progress(DWORD total, DWORD current) {
    if (total > 0) {
        float percentage = (float)current * 100 / total;
        printf("\rDownload progress: %.1f%%", percentage);
    } else {
        printf("\rBytes downloaded: %lu", current);
    }
    fflush(stdout);
}

BOOL parse_url(const wchar_t* url, wchar_t* host, size_t host_len, 
               wchar_t* path, size_t path_len, WORD* port) {
    URL_COMPONENTS url_comp = { 0 };
    url_comp.dwStructSize = sizeof(url_comp);
    url_comp.lpszHostName = host;
    url_comp.dwHostNameLength = host_len;
    url_comp.lpszUrlPath = path;
    url_comp.dwUrlPathLength = path_len;
    url_comp.dwSchemeLength = 1;
    url_comp.dwUserNameLength = 1;
    url_comp.dwPasswordLength = 1;
    url_comp.dwUrlPathLength = 1;
    url_comp.dwExtraInfoLength = 1;

    if (!WinHttpCrackUrl(url, 0, 0, &url_comp)) {
        return FALSE;
    }

    *port = url_comp.nPort;
    return TRUE;
}

DownloadStatus download_file(const DownloadOptions* options) {
    if (!options || !options->url || !options->filename) {
        return DOWNLOAD_INVALID_ARGS;
    }

    HINTERNET hSession = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    FILE* file = NULL;
    BYTE* buffer = NULL;
    DownloadStatus status = DOWNLOAD_SUCCESS;

    // Initialize buffer
    buffer = (BYTE*)malloc(DEFAULT_BUFFER_SIZE);
    if (!buffer) {
        return DOWNLOAD_MEMORY_ERROR;
    }

    // Parse URL
    wchar_t host[MAX_HOST_LENGTH] = {0};
    wchar_t path[MAX_PATH_LENGTH] = {0};
    WORD port = 0;

    if (!parse_url(options->url, host, MAX_HOST_LENGTH, path, MAX_PATH_LENGTH, &port)) {
        free(buffer);
        return DOWNLOAD_INVALID_ARGS;
    }

    do {
        // Initialize WinHTTP
        hSession = WinHttpOpen(L"WinHTTP Downloader/1.0",
                             WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                             WINHTTP_NO_PROXY_NAME,
                             WINHTTP_NO_PROXY_BYPASS,
                             0);
        if (!hSession) {
            status = DOWNLOAD_CONNECT_FAILED;
            break;
        }

        // Set timeouts
        if (options->timeout > 0) {
            WinHttpSetTimeouts(hSession, options->timeout, options->timeout,
                             options->timeout, options->timeout);
        }

        // Connect to server
        hConnect = WinHttpConnect(hSession, host, port, 0);
        if (!hConnect) {
            status = DOWNLOAD_CONNECT_FAILED;
            break;
        }

        // Create request
        hRequest = WinHttpOpenRequest(hConnect, L"GET", path,
                                    NULL, WINHTTP_NO_REFERER,
                                    WINHTTP_DEFAULT_ACCEPT_TYPES,
                                    (port == 443) ? WINHTTP_FLAG_SECURE : 0);
        if (!hRequest) {
            status = DOWNLOAD_REQUEST_FAILED;
            break;
        }

        // Send request
        if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                               WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
            status = DOWNLOAD_REQUEST_FAILED;
            break;
        }

        // Receive response
        if (!WinHttpReceiveResponse(hRequest, NULL)) {
            status = DOWNLOAD_REQUEST_FAILED;
            break;
        }

        // Open file for writing
        if (_wfopen_s(&file, options->filename, L"wb") != 0 || !file) {
            status = DOWNLOAD_WRITE_FAILED;
            break;
        }

        // Get content length
        DWORD contentLength = 0;
        DWORD size = sizeof(contentLength);
        DWORD downloaded = 0;
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
                           WINHTTP_HEADER_NAME_BY_INDEX, &contentLength,
                           &size, WINHTTP_NO_HEADER_INDEX);

        // Download loop
        DWORD bytesRead = 0;
        while (WinHttpReadData(hRequest, buffer, DEFAULT_BUFFER_SIZE, &bytesRead)) {
            if (bytesRead == 0) {
                break;  // Download complete
            }

            // Write to file
            if (fwrite(buffer, 1, bytesRead, file) != bytesRead) {
                status = DOWNLOAD_WRITE_FAILED;
                break;
            }

            downloaded += bytesRead;
            if (options->show_progress) {
                show_download_progress(contentLength, downloaded);
            }
        }

        if (options->show_progress) {
            printf("\nDownload complete!\n");
        }

    } while (0);

    // Cleanup
    if (file) fclose(file);
    if (buffer) free(buffer);
    if (hRequest) WinHttpCloseHandle(hRequest);
    if (hConnect) WinHttpCloseHandle(hConnect);
    if (hSession) WinHttpCloseHandle(hSession);

    return status;
}

const char* get_download_error(DownloadStatus status) {
    switch (status) {
        case DOWNLOAD_SUCCESS:
            return "Success";
        case DOWNLOAD_CONNECT_FAILED:
            return "Connection failed";
        case DOWNLOAD_REQUEST_FAILED:
            return "Request failed";
        case DOWNLOAD_WRITE_FAILED:
            return "File write failed";
        case DOWNLOAD_MEMORY_ERROR:
            return "Memory allocation failed";
        case DOWNLOAD_INVALID_ARGS:
            return "Invalid arguments";
        default:
            return "Unknown error";
    }
}