# File Server System

A cross-platform multi-user file server built in C++ using POSIX sockets, 
multithreading, and Qt GUI framework.

## Features
- Multi-user simultaneous access using threads
- Secure login with role-based access (admin/user)
- File upload and download with chunked transfer
- File management: list, delete, rename
- File locking with mutex to prevent concurrent access corruption
- Qt-based GUI client

## Technologies
- C++17
- POSIX Sockets (TCP)
- std::thread (multithreading)
- std::mutex (file locking)
- Qt5 (GUI)
- Linux / WSL2

## Project Structure
file-server/
├── server/
│   ├── server.cpp      # Main server with thread handling
│   ├── auth.h          # User authentication and AuthManager
│   ├── filemanager.h   # File operations (upload, download, delete, rename)
│   └── filelock.h      # File locking with mutex
├── client/
│   └── client.cpp      # Terminal based client
├── gui/
│   ├── main.cpp        # GUI entry point
│   ├── mainwindow.h/cpp # Main GUI window
│   ├── fileclient.h/cpp # Network client for GUI
│   └── gui.pro         # Qt project file
└── README.md

## How to Run

### Server
```bash
cd server
g++ server.cpp -o server -pthread -lstdc++fs
./server
```

### Terminal Client
```bash
cd client
g++ client.cpp -o client -lstdc++fs
./client
```

### GUI Client
```bash
cd gui
qmake gui.pro
make
./fileclient
```

## Default Users
| Username | Password | Role  |
|----------|----------|-------|
| admin    | admin123 | admin |
| vedant   | vedant123| user  |

## Architecture