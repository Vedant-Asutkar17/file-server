#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string>
#include <sstream>
#include "auth.h"
#include "filemanager.h"

// Global objects shared by all threads
AuthManager auth;
FileManager fileManager("uploads");

// Send message to client
void sendMessage(int socket, std::string message) {
    send(socket, message.c_str(), message.size(), 0);
}

// Receive message from client
std::string receiveMessage(int socket) {
    char buffer[1024] = {0};
    int bytes = recv(socket, buffer, sizeof(buffer), 0);
    if (bytes <= 0) return "";
    std::string msg(buffer);
    if (!msg.empty() && msg.back() == '\n') {
        msg.pop_back();
    }
    return msg;
}

// Handle login
std::string handleLogin(int client_socket) {
    for (int attempt = 1; attempt <= 3; attempt++) {
        sendMessage(client_socket, "Username: ");
        std::string username = receiveMessage(client_socket);

        sendMessage(client_socket, "Password: ");
        std::string password = receiveMessage(client_socket);

        std::string role = auth.login(username, password);

        if (!role.empty()) {
            std::string success = "\nWelcome " + username + " [" + role + "]\n";
            sendMessage(client_socket, success);
            std::cout << "[+] Login: " << username << " (" << role << ")" << std::endl;
            return username;
        } else {
            std::string fail = "Wrong credentials. Attempt " +
                               std::to_string(attempt) + "/3\n";
            sendMessage(client_socket, fail);
        }
    }
    sendMessage(client_socket, "Too many attempts. Disconnecting.\n");
    return "";
}

// Handle one client
void handleClient(int client_socket) {

    sendMessage(client_socket, "=== File Server ===\n");

    // Login
    std::string username = handleLogin(client_socket);
    if (username.empty()) {
        close(client_socket);
        return;
    }

    // Menu loop
    while (true) {
        std::string menu = "\n--- Menu ---\n"
                           "1. List files\n"
                           "2. Upload file\n"
                           "3. Download file\n"
                           "4. Logout\n"
                           "Choice: ";
        sendMessage(client_socket, menu);

        std::string choice = receiveMessage(client_socket);

        // Option 1: List files
        if (choice == "1") {
            std::string files = fileManager.listFiles();
            sendMessage(client_socket, files);

        // Option 2: Upload file
        } else if (choice == "2") {
            sendMessage(client_socket, "Enter filename to upload: ");
            std::string filename = receiveMessage(client_socket);

            sendMessage(client_socket, "Enter file size in bytes: ");
            std::string sizeStr = receiveMessage(client_socket);
            long filesize = std::stol(sizeStr);

            sendMessage(client_socket, "READY");

            bool success = fileManager.receiveFile(client_socket, filename, filesize);

            if (success) {
                sendMessage(client_socket, "\nUpload successful!\n");
            } else {
                sendMessage(client_socket, "\nUpload failed.\n");
            }

        // Option 3: Download file
        } else if (choice == "3") {
            sendMessage(client_socket, "Enter filename to download: ");
            std::string filename = receiveMessage(client_socket);

            fileManager.sendFile(client_socket, filename);

        // Option 4: Logout
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
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;
    bind(server_socket, (sockaddr*)&server_address, sizeof(server_address));

    listen(server_socket, 10);
    std::cout << "=== File Server Started ===" << std::endl;
    std::cout << "Listening on port 9999..." << std::endl;

    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);
        std::thread t(handleClient, client_socket);
        t.detach();
    }

    close(server_socket);
    return 0;
}