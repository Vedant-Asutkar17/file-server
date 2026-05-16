#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <sys/socket.h>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

class FileManager {
private:
    std::string upload_dir;

public:
    // Constructor — set upload directory
    FileManager(std::string dir) {
        upload_dir = dir;
        // Create directory if it doesn't exist
        fs::create_directories(upload_dir);
        std::cout << "[FileManager] Upload dir: " << upload_dir << std::endl;
    }

    // UPLOAD — receive file from client and save to disk
    bool receiveFile(int client_socket, std::string filename, long filesize) {

        // Build full path
        std::string filepath = upload_dir + "/" + filename;

        // Open file for writing
        std::ofstream outfile(filepath, std::ios::binary);
        if (!outfile.is_open()) {
            std::cout << "[Error] Cannot create file: " << filepath << std::endl;
            return false;
        }

        // Receive file in chunks
        char buffer[4096];
        long received = 0;

        while (received < filesize) {

            // Calculate how much to receive this time
            int to_receive = std::min((long)sizeof(buffer), filesize - received);

            // Receive chunk
            int bytes = recv(client_socket, buffer, to_receive, 0);
            if (bytes <= 0) {
                std::cout << "[Error] Connection lost during upload" << std::endl;
                outfile.close();
                return false;
            }

            // Write chunk to file
            outfile.write(buffer, bytes);
            received += bytes;

            // Show progress
            int progress = (int)((received * 100) / filesize);
            std::cout << "\r[Upload] " << filename 
                      << " " << progress << "%" << std::flush;
        }

        outfile.close();
        std::cout << std::endl;
        std::cout << "[Upload] Complete: " << filename 
                  << " (" << filesize << " bytes)" << std::endl;
        return true;
    }

    // DOWNLOAD — send file from disk to client
    bool sendFile(int client_socket, std::string filename) {

        std::string filepath = upload_dir + "/" + filename;

        // Check file exists
        if (!fs::exists(filepath)) {
            std::string err = "ERROR:File not found";
            send(client_socket, err.c_str(), err.size(), 0);
            return false;
        }

        // Get file size
        long filesize = fs::file_size(filepath);

        // Send file info to client first
        std::string info = "OK:" + std::to_string(filesize);
        send(client_socket, info.c_str(), info.size(), 0);

        // Small delay to let client process the info
        usleep(100000);

        // Open file for reading
        std::ifstream infile(filepath, std::ios::binary);
        if (!infile.is_open()) {
            return false;
        }

        // Send file in chunks
        char buffer[4096];
        long sent = 0;

        while (sent < filesize) {
            infile.read(buffer, sizeof(buffer));
            int bytes_read = infile.gcount();

            if (bytes_read <= 0) break;

            send(client_socket, buffer, bytes_read, 0);
            sent += bytes_read;

            // Show progress
            int progress = (int)((sent * 100) / filesize);
            std::cout << "\r[Download] " << filename 
                      << " " << progress << "%" << std::flush;
        }

        infile.close();
        std::cout << std::endl;
        std::cout << "[Download] Complete: " << filename 
                  << " (" << filesize << " bytes)" << std::endl;
        return true;
    }

    // LIST — return all files in upload directory
    std::string listFiles() {
        std::string result = "";
        int count = 0;

        for (auto& entry : fs::directory_iterator(upload_dir)) {
            if (entry.is_regular_file()) {
                std::string name = entry.path().filename().string();
                long size = entry.file_size();
                result += name + " (" + std::to_string(size) + " bytes)\n";
                count++;
            }
        }

        if (count == 0) {
            result = "No files on server.\n";
        }

        return result;
    }
};

#endif