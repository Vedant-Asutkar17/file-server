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
[GUI Client / Terminal Client]
↕ TCP Socket (port 9999)
[File Server]
↕
[File Storage]
## What I Learned
- How TCP sockets work at the OS level
- How to handle multiple clients using threads
- How mutex prevents race conditions
- How files are transferred as binary chunks over a network
- Object oriented design with C++ classes
## Docker Deployment

### Run Server with Docker (No installation needed)

```bash
# Pull and run server
docker pull YOUR_USERNAME/file-server:latest
docker run -d --name myfileserver -p 9999:9999 YOUR_USERNAME/file-server:latest

# Check server is running
docker ps

# View server logs
docker logs myfileserver

# Stop server
docker stop myfileserver
```

### Build Docker image locally

```bash
git clone https://github.com/Vedant-Asutkar17/file-server.git
cd file-server
docker build -t file-server .
docker run -d --name myfileserver -p 9999:9999 file-server
```
## Run with Docker

```bash
docker pull vedant17111/file-server:latest
docker run -d -p 9999:9999 vedant17111/file-server:latest
```

## Docker Hub
https://hub.docker.com/r/vedant17111/file-server