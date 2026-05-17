#ifndef FILELOCK_H
#define FILELOCK_H

#include <iostream>
#include <string>
#include <map>
#include <mutex>

class FileLockManager {
private:
    // Map of filename to mutex
    // Each file has its own mutex
    std::map<std::string, std::mutex*> locks;

    // Master mutex to protect the locks map itself
    std::mutex master_lock;

public:

    // Lock a file
    // Returns true if locked successfully
    // Returns false if file is already locked
    bool lockFile(std::string filename) {
        // Lock the master first to safely access the map
        std::lock_guard<std::mutex> guard(master_lock);

        // If no mutex exists for this file create one
        if (locks.find(filename) == locks.end()) {
            locks[filename] = new std::mutex();
        }

        // Try to lock — non blocking
        bool locked = locks[filename]->try_lock();

        if (locked) {
            std::cout << "[Lock] Locked: " << filename << std::endl;
        } else {
            std::cout << "[Lock] Already locked: " << filename << std::endl;
        }

        return locked;
    }

    // Unlock a file
    void unlockFile(std::string filename) {
        std::lock_guard<std::mutex> guard(master_lock);

        if (locks.find(filename) != locks.end()) {
            locks[filename]->unlock();
            std::cout << "[Lock] Unlocked: " << filename << std::endl;
        }
    }

    // Check if file is locked
    bool isLocked(std::string filename) {
        std::lock_guard<std::mutex> guard(master_lock);

        if (locks.find(filename) == locks.end()) {
            return false;
        }

        // Try to lock — if successful it was not locked
        bool could_lock = locks[filename]->try_lock();
        if (could_lock) {
            locks[filename]->unlock();
            return false;
        }
        return true;
    }

    // Destructor — clean up mutexes
    ~FileLockManager() {
        for (auto& pair : locks) {
            delete pair.second;
        }
    }
};

#endif