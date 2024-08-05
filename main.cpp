#include <iostream>
#include <fstream>
#include <string>

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

int main() {
    std::string configFilePath = "config.txt";
    std::string driveLetter, networkPath;

    if (!readConfig(configFilePath, driveLetter, networkPath)) {
        std::cerr << "Invalid configuration. Please check your config file." << std::endl;
        return 1;
    }

    std::cout << "Drive Letter: " << driveLetter << std::endl;
    std::cout << "Network Path: " << networkPath << std::endl;

    // Simulate functionality
    std::cout << "Simulating drive mapping..." << std::endl;
    std::cout << "Drive " << driveLetter << " mapped to " << networkPath << " successfully." << std::endl;

    return 0;
}
