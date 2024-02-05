#include "ClientFileHandler.h"

void FileHandler::sendFile(SOCKET clientSocket, const char* fileName) {
    const int bufferSize = 1024;
    char buffer[bufferSize];
    std::ifstream inputFile(fileName, std::ios::binary);
    inputFile.seekg(0, std::ios::end);
    int totalSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);

    std::string fileNameStr(fileName);
    int fileNameLength = fileNameStr.size();
    send(clientSocket, reinterpret_cast<char*>(&fileNameLength), sizeof(int), 0);
    send(clientSocket, fileNameStr.c_str(), fileNameLength, 0);

    send(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
    int totalBytesSent = 0;
    while (inputFile) {
        inputFile.read(buffer, sizeof(buffer));
        int bytesRead = inputFile.gcount();
        if (bytesRead > 0) {
            int bytesSent = 0;
            while (bytesSent < bytesRead) {
                int result = send(clientSocket, buffer + bytesSent, bytesRead - bytesSent, 0);
                if (result == SOCKET_ERROR) {
                    std::cerr << "send failed with error: " << WSAGetLastError() << std::endl;
                    closesocket(clientSocket);
                    WSACleanup();
                    return;
                }
                bytesSent += result;
            }
            totalBytesSent += bytesRead;
        }
    }
    inputFile.close();
}

std::string FileHandler::receiveCommand(SOCKET clientSocket) {
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

void FileHandler::listFiles(SOCKET clientSocket) {
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

void FileHandler::showDeleteInfo(SOCKET clientSocket) {
    std::string command;
    command = FileHandler::receiveCommand(clientSocket);
    std::cout << command << std::endl;
}

void FileHandler::showFileInfo(SOCKET clientSocket) {
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
