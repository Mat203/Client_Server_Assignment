#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <Ws2tcpip.h>
#include <string>

// Linking the library needed for network communication
#pragma comment(lib, "ws2_32.lib")

class FileHandler {
public:
    static void sendFile(SOCKET clientSocket, const char* fileName)
    {
        char buffer[2048];
        std::ifstream inputFile(fileName, std::ios::binary);
        inputFile.seekg(0, std::ios::end);
        int totalSize = inputFile.tellg();
        inputFile.seekg(0, std::ios::beg);
        send(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
        int totalBytesSent = 0;
        while (inputFile)
        {
            inputFile.read(buffer, sizeof(buffer));
            int bytesRead = inputFile.gcount();
            if (bytesRead > 0)
            {
                send(clientSocket, buffer, bytesRead, 0);
                totalBytesSent += bytesRead;
                //std::cout << "Sent " << bytesRead << " bytes, total: " << totalBytesSent << " bytes" << std::endl;
            }
        }
        inputFile.close();
    }


    static void receiveFile(SOCKET clientSocket) {
        int totalSize;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "Error in receiving total size." << std::endl;
            return;
        }
        char buffer[1024];
        std::ofstream outputFile("received_file.txt", std::ios::binary);

        int totalReceived = 0;
        while (totalReceived < totalSize)
        {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0)
            {
                outputFile.write(buffer, bytesReceived);
                totalReceived += bytesReceived;
            }
            else
            {
                break;
            }
        }
        outputFile.close();
    }

    static void listFiles(SOCKET clientSocket) {
        std::string command;
        std::cout << "List of files in the directory" << std::endl;
        while (true) {
            command = FileHandler::receiveCommand(clientSocket);
            if (command.find("END") != std::string::npos) {
                break;
            }
            std::cout << command << std::endl;
        }
    }
    static void showFileInfo(SOCKET clientSocket) {
        std::string command;
        std::cout << "File information:" << std::endl;
        while (true) {
            command = FileHandler::receiveCommand(clientSocket);
            std::cout << command << std::endl;
            if (command.find("------------") != std::string::npos) {
                break;
            }
        }
    }
    static void showDeleteInfo(SOCKET clientSocket) {
        std::string command;
        command = FileHandler::receiveCommand(clientSocket);
        std::cout << command << std::endl;

    }

    static std::string receiveCommand(SOCKET clientSocket) {
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            return std::string(buffer, bytesReceived);
        }
        else {
            return "";
        }
    }
};

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
    // Client configuration
    int port = 12345;
    PCWSTR serverIp = L"127.0.0.1";
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    InetPton(AF_INET, serverIp, &serverAddr.sin_addr);
    // Connect to the server
    if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    //Set up server configuration in the separate class

    while (true) {
        std::string command;
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        send(clientSocket, command.c_str(), command.size() + 1, 0);

        if (command.substr(0, 4) == "send") {
            std::string fileName = command.substr(5);
            FileHandler::sendFile(clientSocket, fileName.c_str());
        }
        else if (command.substr(0, 6) == "delete") {
            FileHandler::showDeleteInfo(clientSocket);
        }
        else if (command.substr(0, 7) == "receive") {
            FileHandler::receiveFile(clientSocket);
        }
        else if (command.substr(0, 4) == "list") {
            FileHandler::listFiles(clientSocket);
        }
        else if (command.substr(0, 4) == "info") {
            FileHandler::showFileInfo(clientSocket);
        }
        else if (command == "exit") {
            break;
        }
        else {
            std::cout << "Unknown command" << std::endl;
        }
    }

    // Clean up
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
