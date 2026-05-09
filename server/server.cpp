#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string>
#include "auth.h"

// Global auth manager — shared by all threads
AuthManager auth;

// Send a message to client
void sendMessage(int socket, std::string message) {
    send(socket, message.c_str(), message.size(), 0);
}

// Receive a message from client
std::string receiveMessage(int socket) {
    char buffer[1024] = {0};
    int bytes = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return "";
    std::string msg(buffer);
    // Remove newline
    if (!msg.empty() && msg.back() == '\n') {
        msg.pop_back();
    }
    return msg;
}

// Handle login for a client
// Returns username if login successful, empty string if failed
std::string handleLogin(int client_socket) {

    // Give client 3 attempts
    for (int attempt = 1; attempt <= 3; attempt++) {

        sendMessage(client_socket, "Username: ");
        std::string username = receiveMessage(client_socket);

        sendMessage(client_socket, "Password: ");
        std::string password = receiveMessage(client_socket);

        // Check credentials
        std::string role = auth.login(username, password);

        if (!role.empty()) {
            // Login successful
            std::string success = "\nLogin successful! Welcome " + 
                                   username + " [" + role + "]\n";
            sendMessage(client_socket, success);
            std::cout << "[+] Login success: " << username 
                      << " (" << role << ")" << std::endl;
            return username;
        } else {
            // Login failed
            std::string fail = "Invalid credentials. Attempt " + 
                               std::to_string(attempt) + "/3\n";
            sendMessage(client_socket, fail);
            std::cout << "[-] Login failed for: " << username << std::endl;
        }
    }

    // All attempts used
    sendMessage(client_socket, "Too many failed attempts. Disconnecting.\n");
    return "";
}

// Handle one client in its own thread
void handleClient(int client_socket) {

    std::cout << "[+] New client connected" << std::endl;

    sendMessage(client_socket, "=== File Server ===\n");

    // Step 1: Login
    std::string username = handleLogin(client_socket);

    // If login failed, disconnect
    if (username.empty()) {
        close(client_socket);
        return;
    }

    // Step 2: Show menu after login
    while (true) {
        std::string menu = "\n--- Menu ---\n"
                           "1. List files\n"
                           "2. Upload file\n"
                           "3. Download file\n"
                           "4. Logout\n"
                           "Choice: ";
        sendMessage(client_socket, menu);

        std::string choice = receiveMessage(client_socket);

        if (choice == "1") {
            sendMessage(client_socket, "File list coming soon...\n");

        } else if (choice == "2") {
            sendMessage(client_socket, "Upload coming soon...\n");

        } else if (choice == "3") {
            sendMessage(client_socket, "Download coming soon...\n");

        } else if (choice == "4") {
            sendMessage(client_socket, "Goodbye " + username + "!\n");
            std::cout << "[-] " << username << " logged out" << std::endl;
            break;

        } else {
            sendMessage(client_socket, "Invalid choice.\n");
        }
    }

    close(client_socket);
}


int main() {

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind to port
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket, (sockaddr*)&server_address, sizeof(server_address));

    // Listen
    listen(server_socket, 10);
    std::cout << "=== File Server Started ===" << std::endl;
    std::cout << "Listening on port 9999..." << std::endl;

    // Accept clients forever
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        std::thread t(handleClient, client_socket);
        t.detach();
    }

    close(server_socket);
    return 0;
}