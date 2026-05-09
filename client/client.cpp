#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

int main() {

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    // Connect to server
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    if (connect(client_socket, (sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cout << "Connection failed!" << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    char buffer[1024] = {0};

    // Keep communicating until we quit
    while (true) {

        // Receive message from server
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes <= 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }

        std::cout << buffer << std::endl;

        // Send our response
        std::string input;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        send(client_socket, input.c_str(), input.size(), 0);

        // If we said quit, stop
        if (input == "2") {

            // Receive goodbye message
            memset(buffer, 0, sizeof(buffer));
            recv(client_socket, buffer, sizeof(buffer), 0);
            std::cout << buffer << std::endl;
            break;
        }
    }

    close(client_socket);
    std::cout << "Disconnected." << std::endl;
    return 0;
}