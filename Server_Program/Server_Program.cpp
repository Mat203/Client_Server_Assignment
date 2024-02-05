#include <iostream>
#include <WinSock2.h>
#include <direct.h>
#include <string>
#include <thread>
#include <fstream>
#include <mutex>
#include <vector>
#include "FileHandler.h"

#pragma comment(lib, "ws2_32.lib")

class ClientHandler {
public:
    ClientHandler(SOCKET clientSocket) : clientSocket(clientSocket), fileHandler(clientSocket) {}

    void handle() {
        char buffer[1024];
        memset(buffer, 0, 1024);
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        std::string username;
        if (bytesReceived > 0) {
            username = std::string(buffer, 0, bytesReceived);
            _mkdir(username.c_str());
            std::string message = "Hello " + username;
            send(clientSocket, message.c_str(), message.size() + 1, 0);
        }

        while (true) {
            std::string command = fileHandler.receiveCommand();
            std::cout << command << std::endl;

            if (command.substr(0, 7) == "receive") {
                std::string fileName = command.substr(8);
                fileHandler.sendFile(username, fileName.c_str());
            }
            else if (command.substr(0, 6) == "delete") {
                std::string fileNameToDelete = command.substr(7);
                fileHandler.deleteFile(username, fileNameToDelete.c_str());
            }
            else if (command.substr(0, 4) == "send") {
                fileHandler.receiveFile(username);
            }
            else if (command.substr(0, 4) == "list") {
                fileHandler.listFilesInDirectory(username);
            }
            if (command.substr(0, 4) == "info") {
                std::string fileName = command.substr(5);
                fileHandler.getFileInfo(username, fileName.c_str());
            }
        }

        closesocket(clientSocket);
    }

private:
    SOCKET clientSocket;
    FileHandler fileHandler;
};

class Server {
public:
    Server(int port) : port(port) {
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            exit(1);
        }

        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
            WSACleanup();
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }

        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }

        std::cout << "Server listening on port " << port << std::endl;
    }

    ~Server() {
        closesocket(serverSocket);
        WSACleanup();
    }

    SOCKET acceptClient() {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            exit(1);
        }
        return clientSocket;
    }

private:
    WSADATA wsaData;
    int port;
    SOCKET serverSocket;
    sockaddr_in serverAddr;
};

int main() {
    Server server(12345);

    while (true) {
        SOCKET clientSocket = server.acceptClient();

        ClientHandler clientHandler(clientSocket);
        std::thread clientThread(&ClientHandler::handle, &clientHandler);
        clientThread.detach();
    }

    return 0;
}
