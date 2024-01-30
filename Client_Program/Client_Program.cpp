#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

class FileHandler {
public:
    static void sendFile(SOCKET clientSocket, const char* fileName)
    {
        char buffer[1024];
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
                std::cout << buffer << std::endl;
                totalBytesSent += bytesRead;
                std::cout << "Sent " << bytesRead << " bytes, total: " << totalBytesSent << " bytes" << std::endl;
            }
        }
        inputFile.close();
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

    static void listFiles(SOCKET clientSocket) {
        char buffer[1024];
        std::cout << "List of files in the directory" << std::endl;
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived > 0) {
                std::string command(buffer, bytesReceived);
                if (command.find("END") != std::string::npos) {
                    break;
                }
                std::cout << command << std::endl;
            }
            else {
                break;
            }
        }
    }

    static void showDeleteInfo(SOCKET clientSocket) {
        std::string command;
        command = FileHandler::receiveCommand(clientSocket);
        std::cout << command << std::endl;
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
};

class Server {
public:
    Server(PCWSTR ip, int port) : ip(ip), port(port) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            exit(1);
        }

        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        InetPton(AF_INET, ip, &serverAddr.sin_addr);

        if (connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            exit(1);
        }
    }

    ~Server() {
        closesocket(clientSocket);
        WSACleanup();
    }

    SOCKET getClientSocket() const {
        return clientSocket;
    }

private:
    WSADATA wsaData;
    PCWSTR ip;
    int port;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
};


int main() {
    Server server(L"127.0.0.1", 12345);
    SOCKET clientSocket = server.getClientSocket();

    std::string username;
    std::cout << "Enter your username: ";
    std::getline(std::cin, username);
    send(clientSocket, username.c_str(), username.size() + 1, 0);

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(clientSocket, buffer, 1024, 0);
    if (bytesReceived > 0) {
        std::cout << "SERVER> " << std::string(buffer, 0, bytesReceived) << std::endl;
    }

    while (true) {
        std::string command;
        std::cout << "Enter command: ";
        std::getline(std::cin, command);
        send(clientSocket, command.c_str(), command.size() + 1, 0);

        if (command.substr(0, 4) == "send") {
            std::string fileName = command.substr(5);
            FileHandler::sendFile(clientSocket, fileName.c_str());
        }
        else if (command.substr(0, 4) == "list") {
            FileHandler::listFiles(clientSocket);
        }
        else if (command.substr(0, 6) == "delete") {
            FileHandler::showDeleteInfo(clientSocket);
        }
        else if (command.substr(0, 4) == "info") {
            FileHandler::showFileInfo(clientSocket);
        }

    }

    return 0;
}
