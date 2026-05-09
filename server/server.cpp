#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

int main() {

    // Step 1: Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }
    std::cout << "Socket created successfully" << std::endl;

    // Step 2: Define server address
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Step 3: Bind socket to port
    bind(server_socket, (sockaddr*)&server_address, sizeof(server_address));
    std::cout << "Socket bound to port 9999" << std::endl;

    // Step 4: Listen for connections
    listen(server_socket, 5);
    std::cout << "Server is listening..." << std::endl;

    // Step 5: Accept a client
    int client_socket = accept(server_socket, nullptr, nullptr);
    std::cout << "Client connected!" << std::endl;

    // Step 6: Send message to client
    std::string message = "Welcome to File Server!";
    send(client_socket, message.c_str(), message.size(), 0);

    // Step 7: Close sockets
    close(client_socket);
    close(server_socket);

    return 0;
}