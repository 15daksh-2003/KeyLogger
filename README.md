# KeyLogger

**Disclaimer:** This project is intended for educational purposes only to demonstrate low-level system programming concepts in C, including Windows Hooks, Sockets (Winsock), and basic client-server interaction. Using keyloggers without explicit, informed consent is illegal and unethical in most jurisdictions. The author assumes no liability for misuse of this code. **DO NOT USE THIS ON ANY SYSTEM YOU DO NOT HAVE EXPLICIT PERMISSION TO MONITOR.**

## Overview

This project implements a simple keylogger using a client-server architecture written in C for the Windows operating system.

*   **Client (`client.c`):** Runs on the target machine. It captures keyboard input using a low-level Windows keyboard hook and sends the captured key data to a remote server over a TCP connection.
*   **Server (`server.c`):** Runs on a separate machine. It listens for incoming connections from clients, receives the keystroke data, and logs it to a file specific to each connected client.

## Functionality

*   **Keystroke Capture:** The client uses `SetWindowsHookExA` with `WH_KEYBOARD_LL` to install a system-wide low-level keyboard hook, intercepting `WM_KEYDOWN` messages.
*   **Client-Server Communication:** Uses standard Windows Sockets (Winsock) for TCP/IP communication between the client and server.
*   **Data Transmission:** The client sends the captured virtual key code (as a raw character representation) to the server immediately upon detection.
*   **Multi-Client Handling:** The server uses multithreading (`_beginthreadex`) to handle connections from multiple clients concurrently.
*   **Data Logging:** The server receives data from each client and appends it to a unique text file. The filename is based on the client's IP address (with '.' replaced by '_') and stored in a predefined directory.

## How it Works

### Client (`client.c`)

1.  **Initialization:** Initializes Winsock.
2.  **Socket Creation:** Creates a TCP socket.
3.  **Server Connection:** Connects to the specified `SERVER_IP` and `PORT`.
4.  **Keyboard Hook Installation:** Installs a low-level keyboard hook (`WH_KEYBOARD_LL`) using `SetWindowsHookExA`. The `HookProc` function is designated as the callback.
5.  **Hook Procedure (`HookProc`):**
    *   This function is called by the system whenever a keyboard event occurs.
    *   It checks if the event is a key press (`WM_KEYDOWN`).
    *   It retrieves the virtual key code (`vkCode`) from the event data (`KBDLLHOOKSTRUCT`).
    *   **Important:** It formats the integer `vkCode` directly into a character using `sprintf(buffer, "%c", pKey->vkCode)`. This is a very basic conversion and **will not accurately represent the typed character** (e.g., it doesn't handle Shift key, Caps Lock, different keyboard layouts, or translate special keys like Enter, Backspace, etc., into meaningful text).
    *   It sends this single-character representation of the `vkCode` to the connected server using `send()`.
    *   It calls `CallNextHookEx` to pass the event to the next hook in the chain.
6.  **Message Loop:** Enters a standard Windows message loop (`GetMessage`, `TranslateMessage`, `DispatchMessage`). This is necessary for the hook procedure to be called correctly.
7.  **Cleanup:** (Theoretically, after the message loop exits) Unhooks the keyboard hook, closes the socket, and cleans up Winsock.

### Server (`server.c`)

1.  **Initialization:** Initializes Winsock.
2.  **Socket Creation:** Creates a TCP listening socket.
3.  **Binding:** Binds the socket to listen on all available network interfaces (`INADDR_ANY`) on the specified `PORT`.
4.  **Listening:** Puts the socket into listening mode (`listen`).
5.  **Accepting Connections:** Enters a loop to `accept` incoming client connections.
6.  **Client Handling:**
    *   When a client connects, `accept` returns a new socket for communication with that specific client.
    *   It retrieves the client's IP address.
    *   It sanitizes the IP address (replacing '.' with '_') for use in a filename.
    *   It constructs a unique log filename (e.g., `C:\PR\C\KeyLogger\userKLData\RemoteFile_192_168_1_10.txt`).
    *   It allocates memory to store client data (socket and address).
    *   It creates a new thread using `_beginthreadex`, passing the `client_handler` function and the client data pointer. This allows the server to handle the new client without blocking further incoming connections.
7.  **Client Handler Thread (`client_handler`):**
    *   This function runs in a separate thread for each connected client.
    *   It opens the client-specific log file in append mode (`"a"`).
    *   It enters a loop to receive data (`recv`) from the client socket.
    *   Received data (the raw character representation of `vkCode` sent by the client) is written directly to the log file using `fprintf`.
    *   `fflush` is called after each write to ensure data is immediately written to the file.
    *   The loop continues until the client disconnects (`recv` returns 0) or an error occurs (`recv` returns `SOCKET_ERROR`).
    *   **Cleanup (Thread):** Closes the log file, closes the client socket, and frees the allocated client data structure.
8.  **Cleanup (Main):** (Theoretically, after the accept loop exits) Closes the main server listening socket and cleans up Winsock.

## Project Structure
├── client.c # Source code for the client application
└── server.c # Source code for the server application

Usage
1.	Compile both client.c and server.c after performing the necessary configuration steps.
2.	Run the compiled server.exe on the designated server machine. It will print messages indicating it's initialized and waiting for connections.
3.	Run the compiled client.exe on the target machine(s) you have permission to monitor. It will attempt to connect to the server and install the keyboard hook.
4.	As keys are pressed on the target machine(s), the server will receive the data and log it into the corresponding RemoteFile_<ip_address>.txt file in the configured directory.
5.	Check the log files on the server machine to view the captured data.

Limitations and Important Notes
•	Inaccurate Logging: The most significant limitation is that the client sends the raw virtual key code (vkCode) converted directly to a character. This does not represent actual typed text. It doesn't account for modifier keys (Shift, Ctrl, Alt), keyboard layouts, Caps Lock state, or translate special keys (Enter, Tab, Backspace, F-keys, etc.) into a readable format. The log file will contain mostly uninterpretable characters based on the ASCII values of the vkCode integers.
•	Non-Stealthy: Both client and server print status messages to the console. They are not hidden processes.
•	Hardcoded Configuration: Server IP in the client and log path in the server are hardcoded.
•	No Encryption: Keystroke data is sent over the network in plain text, making it vulnerable to sniffing.
•	Basic Error Handling: Error handling primarily consists of printing messages to the console. Robust error recovery is not implemented.
•	Antivirus/EDR Detection: Keyloggers, even simple ones like this, are highly likely to be detected and blocked by antivirus software and Endpoint Detection & Response (EDR) systems.
•	Resource Management: While the server uses threads, extremely high numbers of simultaneous connections could potentially strain resources. Basic socket timeouts are set in the client, but the server's recv loop might block indefinitely if no data arrives but the connection stays open.

Ethical Considerations
Again, this project is for educational demonstration only. Unauthorized deployment or use of keylogging software is illegal and unethical.

