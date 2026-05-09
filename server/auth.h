#ifndef AUTH_H
#define AUTH_H

#include <iostream>
#include <string>
#include <map>

// User class — stores one user's information
class User {
public:
    std::string username;
    std::string password;
    std::string role;      // "admin" or "user"

    // Constructor
    User(std::string u, std::string p, std::string r) {
        username = u;
        password = p;
        role     = r;
    }

    // Default constructor
    User() {}
};


// AuthManager class — manages all users
// Handles login, adding users, checking passwords
class AuthManager {
private:
    // Map stores users — key is username, value is User object
    std::map<std::string, User> users;

public:
    // Constructor — add default users when server starts
    AuthManager() {
        // Add a default admin user
        users["admin"] = User("admin", "admin123", "admin");

        // Add a default normal user
        users["vedant"] = User("vedant", "vedant123", "user");

        std::cout << "[Auth] Loaded " << users.size() << " users" << std::endl;
    }

    // Login function
    // Returns role if login successful, empty string if failed
    std::string login(std::string username, std::string password) {

        // Check if username exists
        if (users.find(username) == users.end()) {
            return "";   // User not found
        }

        // Check if password matches
        if (users[username].password == password) {
            return users[username].role;   // Return role
        }

        return "";   // Wrong password
    }

    // Check if a username exists
    bool userExists(std::string username) {
        return users.find(username) != users.end();
    }

    // Get user role
    std::string getRole(std::string username) {
        if (userExists(username)) {
            return users[username].role;
        }
        return "";
    }
};

#endif