#pragma once

#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

class FileHandler {
public:
    static void sendFile(SOCKET clientSocket, const char* fileName);

    static std::string receiveCommand(SOCKET clientSocket);

    static void listFiles(SOCKET clientSocket);

    static void showDeleteInfo(SOCKET clientSocket);

    static void showFileInfo(SOCKET clientSocket);
};
