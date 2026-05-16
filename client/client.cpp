#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void sendMessage(int socket, std::string message) {
    send(socket, message.c_str(), message.size(), 0);
}

std::string receiveMessage(int socket) {
    char buffer[4096] = {0};
    recv(socket, buffer, sizeof(buffer), 0);
    return std::string(buffer);
}

void uploadFile(int client_socket) {

    // Ask user for file path
    std::string filepath;
    std::cout << "Enter full path of file: ";
    std::getline(std::cin, filepath);

    // Check file exists
    if (!fs::exists(filepath)) {
        std::cout << "File not found." << std::endl;
        // Tell server we cancelled
        sendMessage(client_socket, "CANCEL");
        return;
    }

    // Get filename and size automatically
    std::string filename = fs::path(filepath).filename().string();
    long filesize = fs::file_size(filepath);

    std::cout << "File: " << filename << std::endl;
    std::cout << "Size: " << filesize << " bytes" << std::endl;

    // Send filename to server
    sendMessage(client_socket, filename);
    usleep(200000);

    // Send filesize to server
    sendMessage(client_socket, std::to_string(filesize));
    usleep(200000);

    // Wait for READY signal from server
    std::string ready = receiveMessage(client_socket);
    if (ready.find("READY") == std::string::npos) {
        std::cout << "Server not ready. Aborting." << std::endl;
        return;
    }

    // Send file in chunks
    std::ifstream infile(filepath, std::ios::binary);
    char buffer[4096];
    long sent = 0;

    while (sent < filesize) {
        infile.read(buffer, sizeof(buffer));
        int bytes_read = infile.gcount();
        if (bytes_read <= 0) break;

        send(client_socket, buffer, bytes_read, 0);
        sent += bytes_read;

        int progress = (int)((sent * 100) / filesize);
        std::cout << "\rUploading: " << progress << "%" << std::flush;
    }

    infile.close();
    std::cout << "\nDone sending." << std::endl;

    // Get confirmation from server
    std::string response = receiveMessage(client_socket);
    std::cout << response << std::endl;
}

void downloadFile(int client_socket, std::string filename) {

    // Receive file info from server
    std::string info = receiveMessage(client_socket);

    if (info.find("ERROR") != std::string::npos) {
        std::cout << "File not found on server." << std::endl;
        return;
    }

    // Parse filesize — format is "OK:12345"
    long filesize = std::stol(info.substr(3));
    std::cout << "File size: " << filesize << " bytes" << std::endl;
    std::cout << "Downloading..." << std::endl;

    // Save file with same name
    std::ofstream outfile(filename, std::ios::binary);
    char buffer[4096];
    long received = 0;

    while (received < filesize) {
        int to_receive = std::min((long)sizeof(buffer), filesize - received);
        int bytes = recv(client_socket, buffer, to_receive, 0);
        if (bytes <= 0) break;

        outfile.write(buffer, bytes);
        received += bytes;

        int progress = (int)((received * 100) / filesize);
        std::cout << "\rDownloading: " << progress << "%" << std::flush;
    }

    outfile.close();
    std::cout << "\nSaved as: " << filename << std::endl;
}

int main() {

    // Connect to server
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    if (connect(client_socket, (sockaddr*)&server_address,
                sizeof(server_address)) == -1) {
        std::cout << "Connection failed!" << std::endl;
        return 1;
    }

    std::cout << "Connected to server!" << std::endl;

    while (true) {

        // Receive message from server
        char buffer[4096] = {0};
        int bytes = recv(client_socket, buffer, sizeof(buffer), 0);

        if (bytes <= 0) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }

        std::string msg(buffer);
        std::cout << msg;

        // Login prompts — just send input directly
        if (msg.find("Username:") != std::string::npos ||
            msg.find("Password:") != std::string::npos) {
            std::string input;
            std::getline(std::cin, input);
            sendMessage(client_socket, input);
            continue;
        }

        // Main menu
        if (msg.find("Choice:") != std::string::npos) {
            std::string choice;
            std::getline(std::cin, choice);
            sendMessage(client_socket, choice);

            // Upload
            if (choice == "2") {
                uploadFile(client_socket);
            }

            // Download
            if (choice == "3") {
                // Wait for server to ask filename
                char buf2[4096] = {0};
                recv(client_socket, buf2, sizeof(buf2), 0);
                std::cout << buf2;

                std::string filename;
                std::getline(std::cin, filename);
                sendMessage(client_socket, filename);

                downloadFile(client_socket, filename);
            }

            // Logout
            if (choice == "4") {
                char buf2[4096] = {0};
                recv(client_socket, buf2, sizeof(buf2), 0);
                std::cout << buf2 << std::endl;
                break;
            }

            continue;
        }
    }

    close(client_socket);
    return 0;
}