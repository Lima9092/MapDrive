#define SECURITY_WIN32  // Define SECURITY_WIN32 to use Windows security APIs in user mode

#include <iostream>     // Standard input/output stream operations
#include <fstream>      // File operations
#include <string>       // std::string
#include <windows.h>    // Windows API functions and data types
#include <Lmcons.h>     // Constants like UNLEN
#include <wincred.h>    // Windows Credential Manager functions
#include <security.h>   // Security functions
#include <secext.h>     // Security extension functions like GetUserNameEx

// Helper function to convert std::string to std::wstring
std::wstring stringToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

// Function to read the configuration file
bool readConfig(const std::string& filePath, std::string& driveLetter, std::string& networkPath) {
    std::ifstream configFile(filePath);
    if (!configFile.is_open()) {
        std::cerr << "Failed to open configuration file." << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(configFile, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "drive_letter") {
                driveLetter = value;
            } else if (key == "network_path") {
                networkPath = value;
            }
        }
    }

    return !driveLetter.empty() && !networkPath.empty();
}

// Function to map the drive
bool mapDrive(const std::string& driveLetter, const std::string& networkPath, const std::string& username, const std::string& password) {
    NETRESOURCEW nr;
    memset(&nr, 0, sizeof(NETRESOURCEW));
    nr.dwType = RESOURCETYPE_DISK;
    nr.lpLocalName = const_cast<LPWSTR>(stringToWString(driveLetter).c_str());
    nr.lpRemoteName = const_cast<LPWSTR>(stringToWString(networkPath).c_str());

    DWORD result = WNetCancelConnection2W(nr.lpLocalName, 0, TRUE);
    if (result != NO_ERROR && result != ERROR_NOT_CONNECTED) {
        std::cerr << "Warning: Failed to disconnect existing connection on " << driveLetter << " (Error: " << result << ")." << std::endl;
    }

    result = WNetAddConnection2W(&nr, stringToWString(password).c_str(), stringToWString(username).c_str(), 0);
    if (result != NO_ERROR) {
        std::cerr << "Error: Failed to map drive (Error: " << result << ")." << std::endl;
        return false;
    }

    std::cout << "Drive " << driveLetter << " mapped to " << networkPath << " successfully." << std::endl;
    return true;
}

// Function to securely get a password from the user
std::string getPassword() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));

    std::string password;
    std::getline(std::cin, password);

    SetConsoleMode(hStdin, mode);
    std::cout << std::endl; // To simulate a new line after password entry
    return password;
}

// Function to get the full UPN of the current user, or fall back to a standard username
std::string getCurrentUserUPN() {
    wchar_t upn[UNLEN + UNLEN + 1]; // Assuming maximum length
    DWORD upn_len = sizeof(upn) / sizeof(upn[0]);

    if (GetUserNameExW(NameUserPrincipal, upn, &upn_len)) {
        // Convert wstring to string
        std::wstring wideStr(upn, upn_len);
        return std::string(wideStr.begin(), wideStr.end());
    } else {
        std::cerr << "Error: Unable to retrieve the current user's UPN. Using a standard username instead." << std::endl;

        // Fall back to a standard username
        char username[UNLEN + 1];
        DWORD username_len = UNLEN + 1;
        if (GetUserNameA(username, &username_len)) {
            return std::string(username);
        } else {
            std::cerr << "Error: Unable to retrieve the standard username." << std::endl;
            return ""; // Return empty string if all attempts fail
        }
    }
}

// Function to store credentials in Windows Credential Manager
bool storeCredentials(const std::string& targetName, const std::string& username, const std::string& password) {
    CREDENTIALW cred = {0};
    cred.Type = CRED_TYPE_GENERIC;
    cred.TargetName = const_cast<LPWSTR>(stringToWString(targetName).c_str());
    cred.CredentialBlobSize = static_cast<DWORD>(password.size());
    cred.CredentialBlob = (LPBYTE)password.c_str();
    cred.Persist = CRED_PERSIST_ENTERPRISE; // Persist across reboots
    cred.UserName = const_cast<LPWSTR>(stringToWString(username).c_str());

    if (CredWriteW(&cred, 0)) {
        std::cout << "Credentials stored in Credential Manager successfully." << std::endl;
        return true;
    } else {
        std::cerr << "Error: Failed to store credentials in Credential Manager." << std::endl;
        return false;
    }
}

int main() {
    std::string configFilePath = "config.txt";
    std::string driveLetter, networkPath;

    // Read the configuration file
    if (!readConfig(configFilePath, driveLetter, networkPath)) {
        std::cerr << "Invalid configuration. Please check your config file." << std::endl;
        return 1;
    }

    // Get the current user's full UPN or fall back to a standard username
    std::string userUPN = getCurrentUserUPN();
    if (userUPN.empty()) {
        return 1;
    }

    std::cout << "Username: " << userUPN << std::endl;
    std::cout << "Enter your password: ";

    // Prompt the user for their password
    std::string password = getPassword();

    // Store the credentials in Windows Credential Manager
    if (!storeCredentials(networkPath, userUPN, password)) {
        return 1;
    }

    // Attempt to map the drive
    if (!mapDrive(driveLetter, networkPath, userUPN, password)) {
        return 1;
    }

    return 0;
}
