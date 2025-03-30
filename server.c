// Server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <process.h> // For _beginthreadex()
#include <windows.h>

#pragma comment(lib,"ws2_32.lib") // Link with ws2_32.lib

#define PORT 8888
#define BUFFER_SIZE 1024

typedef struct {
    SOCKET socket;
    struct sockaddr_in address;
} CLIENT_DATA;

unsigned __stdcall client_handler(void *data) {
    CLIENT_DATA *client_data = (CLIENT_DATA *)data;
    SOCKET client_socket = client_data->socket;
    struct sockaddr_in client_addr = client_data->address;
    char buffer[BUFFER_SIZE];
    int recv_size;

    FILE *file;
    char filename[260];

    // Get client IP address
    char *client_ip = inet_ntoa(client_addr.sin_addr);

    // Sanitize IP for filename
    char sanitized_ip[20];
    strncpy(sanitized_ip, client_ip, sizeof(sanitized_ip));
    for (char *p = sanitized_ip; *p; ++p) {
        if (*p == '.') *p = '_';
    }

    // Construct the filename
    snprintf(filename, sizeof(filename), "C:\\respective_path\\RemoteFile_%s.txt", sanitized_ip); //change as per the dir structure defined on server machine

    // Open the file
    file = fopen(filename, "a");
    if (file == NULL) {
        printf("Error opening file %s!\n", filename);
        closesocket(client_socket);
        free(client_data);
        return 0;
    }

    // Receive data
    while((recv_size = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[recv_size] = '\0';
        fprintf(file, "%s", buffer);
        fflush(file);
    }

    // Check for errors
    if (recv_size == 0) {
        printf("Client %s disconnected.\n", client_ip);
    } else if (recv_size == SOCKET_ERROR) {
        printf("recv failed with error code: %d\n", WSAGetLastError());
    }

    // Cleanup
    fclose(file);
    closesocket(client_socket);
    free(client_data);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_size;

    // Initialize Winsock
    printf("Initializing Winsock...\n");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0) {
        printf("Failed. Error Code: %d\n",WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create socket
    if((server_socket = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n" , WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");

    // Prepare sockaddr_in
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed with error code: %d\n" , WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Bind done.\n");

    // Listen
    listen(server_socket, SOMAXCONN);
    printf("Waiting for incoming connections...\n");

    client_size = sizeof(struct sockaddr_in);

    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &client_size)) != INVALID_SOCKET) {
        printf("Connection accepted from %s.\n", inet_ntoa(client.sin_addr));
    
        // Allocate memory for client data
        CLIENT_DATA *client_data = (CLIENT_DATA *)malloc(sizeof(CLIENT_DATA));
        if (client_data == NULL) {
            printf("Memory allocation failed.\n");
            closesocket(client_socket);
            continue;
        }

        // Populate client data
        client_data->socket = client_socket;
        client_data->address = client;

        // Create thread
        HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, client_handler, (void*)client_data, 0, NULL);
        if (hThread == NULL) {
            printf("Error creating thread: %d\n", GetLastError());
            closesocket(client_socket);
            free(client_data);
        } else {
            CloseHandle(hThread);
        }
    }    

    // Check for accept error
    if (client_socket == INVALID_SOCKET) {
        printf("accept failed with error code: %d\n", WSAGetLastError());
    }

    // Cleanup
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
