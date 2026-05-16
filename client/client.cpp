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

int sock;

// Send message to server
void sendMsg(std::string msg) {
    send(sock, msg.c_str(), msg.size(), 0);
}

// Receive message from server
std::string recvMsg() {
    char buffer[4096] = {0};
    recv(sock, buffer, sizeof(buffer), 0);
    return std::string(buffer);
}

// Upload file
void uploadFile() {
    // Wait for UPLOAD_START
    std::string signal = recvMsg();
    if (signal.find("UPLOAD_START") == std::string::npos) {
        std::cout << "Server error." << std::endl;
        return;
    }

    // Ask user for file path
    std::string filepath;
    std::cout << "Enter full path of file: ";
    std::getline(std::cin, filepath);

    // Check file exists
    if (!fs::exists(filepath)) {
        std::cout << "File not found." << std::endl;
        sendMsg("CANCEL");
        usleep(100000);
        sendMsg("0");
        return;
    }

    // Get info
    std::string filename = fs::path(filepath).filename().string();
    long filesize = fs::file_size(filepath);
    std::cout << "Uploading: " << filename << " (" << filesize << " bytes)" << std::endl;

    // Send filename and size
    sendMsg(filename);
    usleep(200000);
    sendMsg(std::to_string(filesize));
    usleep(200000);

    // Wait for READY
    std::string ready = recvMsg();
    if (ready.find("READY") == std::string::npos) {
        std::cout << "Server not ready." << std::endl;
        return;
    }

    // Send file chunks
    std::ifstream infile(filepath, std::ios::binary);
    char buffer[4096];
    long sent = 0;
    while (sent < filesize) {
        infile.read(buffer, sizeof(buffer));
        int n = infile.gcount();
        if (n <= 0) break;
        send(sock, buffer, n, 0);
        sent += n;
        std::cout << "\rProgress: " << (sent * 100 / filesize) << "%" << std::flush;
    }
    infile.close();
    std::cout << std::endl;

    // Get result
    std::cout << recvMsg() << std::endl;
}

// Download file
void downloadFile() {
    // Wait for server prompt
    std::cout << recvMsg();

    // Send filename
    std::string filename;
    std::getline(std::cin, filename);
    sendMsg(filename);

    // Get file info
    std::string info = recvMsg();
    if (info.find("ERROR") != std::string::npos) {
        std::cout << "File not found on server." << std::endl;
        return;
    }

    long filesize = std::stol(info.substr(3));
    std::cout << "Size: " << filesize << " bytes. Downloading..." << std::endl;

    // Receive file
    std::ofstream outfile(filename, std::ios::binary);
    char buffer[4096];
    long received = 0;
    while (received < filesize) {
        int n = recv(sock, buffer, std::min((long)sizeof(buffer), filesize - received), 0);
        if (n <= 0) break;
        outfile.write(buffer, n);
        received += n;
        std::cout << "\rProgress: " << (received * 100 / filesize) << "%" << std::flush;
    }
    outfile.close();
    std::cout << "\nSaved: " << filename << std::endl;
}

// Delete file
void deleteFile() {
    std::cout << recvMsg();
    std::string filename;
    std::getline(std::cin, filename);
    sendMsg(filename);
    std::cout << recvMsg() << std::endl;
}

// Rename file
void renameFile() {
    std::cout << recvMsg();
    std::string old_name;
    std::getline(std::cin, old_name);
    sendMsg(old_name);

    std::cout << recvMsg();
    std::string new_name;
    std::getline(std::cin, new_name);
    sendMsg(new_name);

    std::cout << recvMsg() << std::endl;
}

int main() {

    // Connect
    sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9999);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cout << "Connection failed!" << std::endl;
        return 1;
    }
    std::cout << "Connected to server!" << std::endl;

    while (true) {

        // Receive from server
        std::string msg = recvMsg();
        if (msg.empty()) {
            std::cout << "Server disconnected." << std::endl;
            break;
        }

        std::cout << msg;

        // Handle login
        if (msg.find("Username:") != std::string::npos ||
            msg.find("Password:") != std::string::npos ||
            msg.find("Attempt") != std::string::npos) {
            std::string input;
            std::getline(std::cin, input);
            sendMsg(input);
            continue;
        }

        // Handle menu
        if (msg.find("Choice:") != std::string::npos) {
            std::string choice;
            std::getline(std::cin, choice);
            sendMsg(choice);

            if      (choice == "2") uploadFile();
            else if (choice == "3") downloadFile();
            else if (choice == "4") deleteFile();
            else if (choice == "5") renameFile();
            else if (choice == "6") {
                std::cout << recvMsg() << std::endl;
                break;
            }
            continue;
        }
    }

    close(sock);
    return 0;
}