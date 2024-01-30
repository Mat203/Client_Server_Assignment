#include <iostream>
#include <WinSock2.h>
#include <direct.h>
#include <string>
#include <thread>
#include <fstream>

#pragma comment(lib, "ws2_32.lib")

class FileHandler {
public:
    static void receiveFile(SOCKET clientSocket, const std::string& username)
    {
        char buffer[2048];
        std::string directoryPath = username + "/";
        std::string fileName = "received_file.txt";
        std::string fullPath = directoryPath + fileName;
        std::ofstream outputFile(fullPath, std::ios::binary);

        int totalSize;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            std::cout << "Error in receiving total size." << std::endl;
            return;
        }

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

    static void sendFile(SOCKET clientSocket, const std::string& username, const char* fileName)
    {
        char buffer[1024];
        std::string directoryPath = username + "/";
        std::string fullPath = directoryPath + fileName;
        std::ifstream inputFile(fullPath.c_str(), std::ios::binary);
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

    static void listFilesInDirectory(SOCKET clientSocket, const std::string& username) {
        WIN32_FIND_DATAA fileData;
        HANDLE hFind;
        std::cout << "List of files in the client's directory" << std::endl;
        std::string directoryPath = username + "\\*";
        if (!((hFind = FindFirstFileA(directoryPath.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
            do {
                if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::string fileName = fileData.cFileName;
                    fileName += "\n";
                    send(clientSocket, fileName.c_str(), fileName.size() + 1, 0);
                    std::cout << fileName << std::endl;
                }
            } while (FindNextFileA(hFind, &fileData));
            FindClose(hFind);
        }
        else {
            std::cout << "Path not found" << std::endl;
        }
        std::cout << "-----------------------" << std::endl;
        std::string endMarker = "END";
        send(clientSocket, endMarker.c_str(), endMarker.size() + 1, 0);
    }

    static void deleteFile(SOCKET clientSocket, const std::string& username, const char* fileName) {
        std::string directoryPath = username + "/";
        std::string fullPath = directoryPath + fileName;
        if (remove(fullPath.c_str()) != 0) {
            std::cout << "Error deleting file" << std::endl;
        }
        else {
            std::cout << "File successfully deleted" << std::endl;
            std::string successMessage = "File successfully deleted";
            send(clientSocket, successMessage.c_str(), successMessage.size() + 1, 0);
        }
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

    static void getFileInfo(SOCKET clientSocket, const std::string& username, const char* fileName) {
        std::string directoryPath = username + "/";
        std::string fullPath = directoryPath + fileName;
        WIN32_FILE_ATTRIBUTE_DATA fileInfo;

        if (GetFileAttributesExA(fullPath.c_str(), GetFileExInfoStandard, &fileInfo)) {
            LARGE_INTEGER size;
            size.HighPart = fileInfo.nFileSizeHigh;
            size.LowPart = fileInfo.nFileSizeLow;

            FILETIME lastWriteTime = fileInfo.ftLastWriteTime;
            SYSTEMTIME stUTC, stLocal;
            FileTimeToSystemTime(&lastWriteTime, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            std::string timeInfo = "Last modified: " + std::to_string(stLocal.wDay) + "/" + std::to_string(stLocal.wMonth) + "/" + std::to_string(stLocal.wYear) + " " + std::to_string(stLocal.wHour) + ":" + std::to_string(stLocal.wMinute) + "\n";
            send(clientSocket, timeInfo.c_str(), timeInfo.size() + 1, 0);

            std::string sizeInfo = "Size: " + std::to_string(size.QuadPart) + " bytes\n";
            send(clientSocket, sizeInfo.c_str(), sizeInfo.size() + 1, 0);

            std::string endMarker = "------------\n";
            send(clientSocket, endMarker.c_str(), endMarker.size() + 1, 0);

        }
        else {
            std::string error = "Failed to get file attributes.\n";
            send(clientSocket, error.c_str(), error.size() + 1, 0);
        }
    }
};

void handleClient(SOCKET clientSocket) {
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
        std::string command = FileHandler::receiveCommand(clientSocket);
        std::cout << command << std::endl;

        if (command.substr(0, 7) == "receive") {
            std::string fileName = command.substr(8);
            FileHandler::sendFile(clientSocket, username, fileName.c_str());
        }
        else if (command.substr(0, 6) == "delete") {
            std::string fileNameToDelete = command.substr(7);
            FileHandler::deleteFile(clientSocket, username,fileNameToDelete.c_str());
        }
        else if (command.substr(0, 4) == "send") {
            FileHandler::receiveFile(clientSocket, username);
        }
        else if (command.substr(0, 4) == "list") {
            FileHandler::listFilesInDirectory(clientSocket, username);
        }
        if (command.substr(0, 4) == "info") {
            std::string fileName = command.substr(5);
            FileHandler::getFileInfo(clientSocket, username, fileName.c_str());
        }
    }

    closesocket(clientSocket);
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }

    int port = 12345;
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
