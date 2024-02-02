#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <WinSock2.h>
#include <Windows.h>
#include <mutex>

class FileHandler {
public:
    static void receiveFile(SOCKET clientSocket, const std::string& username);
    static void sendFile(SOCKET clientSocket, const std::string& username, const char* fileName);
    static void listFilesInDirectory(SOCKET clientSocket, const std::string& username);
    static void deleteFile(SOCKET clientSocket, const std::string& username, const char* fileName);
    static std::string receiveCommand(SOCKET clientSocket);
    static void getFileInfo(SOCKET clientSocket, const std::string& username, const char* fileName);

private:
    static std::mutex m;
};
