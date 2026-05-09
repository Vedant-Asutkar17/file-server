#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string>

// This function handles ONE client
// Each client gets their own copy of this running in a thread
void handleClient(int client_socket) {

    // Step 1: Ask for name
    std::string welcome = "Enter your name: ";
    send(client_socket, welcome.c_str(), welcome.size(), 0);

    // Step 2: Receive name
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer), 0);
    std::string username(buffer);

    // Remove newline character if present
    if (!username.empty() && username.back() == '\n') {
        username.pop_back();
    }

    std::cout << "[+] User connected: " << username << std::endl;

    // Step 3: Keep talking to this client until they quit
    while (true) {

        // Ask what they want to do
        std::string menu = "\nWhat do you want to do?\n"
                           "1. Say hello\n"
                           "2. Quit\n"
                           "Choice: ";
        send(client_socket, menu.c_str(), menu.size(), 0);

        // Receive their choice
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        // If recv returns 0 or less, client disconnected
        if (bytes <= 0) {
            break;
        }

        std::string choice(buffer);

        // Remove newline
        if (!choice.empty() && choice.back() == '\n') {
            choice.pop_back();
        }

        // Handle their choice
        if (choice == "1") {
            std::string response = "Hello " + username + "! Welcome to the file server!";
            send(client_socket, response.c_str(), response.size(), 0);

        } else if (choice == "2") {
            std::string bye = "Goodbye " + username + "!";
            send(client_socket, bye.c_str(), bye.size(), 0);
            break;

        } else {
            std::string invalid = "Invalid choice. Try again.";
            send(client_socket, invalid.c_str(), invalid.size(), 0);
        }
    }

    // Step 4: Clean up
    close(client_socket);
    std::cout << "[-] User disconnected: " << username << std::endl;
}


int main() {

    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // This lets us reuse the port immediately after restart
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
    std::cout << "Waiting for clients..." << std::endl;

    // Keep accepting clients forever
    while (true) {
        int client_socket = accept(server_socket, nullptr, nullptr);

        std::cout << "[+] New connection accepted" << std::endl;

        // Spawn a new thread for this client
        // Each client runs independently
        std::thread t(handleClient, client_socket);

        // Detach thread so it runs on its own
        // We don't wait for it to finish
        t.detach();
    }

    close(server_socket);
    return 0;
}