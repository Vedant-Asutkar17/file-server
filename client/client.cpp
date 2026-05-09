#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {

    // Step 1: Create a socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Socket created" << std::endl;

    // Step 2: Define server address to connect to
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    // Step 3: Connect to server
    connect(client_socket, (sockaddr*)&server_address, sizeof(server_address));
    std::cout << "Connected to server" << std::endl;

    // Step 4: Receive message from server
    char buffer[1024] = {0};
    recv(client_socket, buffer, sizeof(buffer), 0);
    std::cout << "Server says: " << buffer << std::endl;

    // Step 5: Close socket
    close(client_socket);

    return 0;
}