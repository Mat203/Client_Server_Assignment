#pragma once
#include <WinSock2.h>
#include <string>
#include <mutex>
#include <iostream>
#include <vector>
#include <fstream>
#include <WinSock2.h>
#include <string>

class FileHandler {
public:
    FileHandler(SOCKET clientSocket);

    void receiveFile(const std::string& username);
    void sendFile(const std::string& username, const char* fileName);
    void listFilesInDirectory(const std::string& username);
    void deleteFile(const std::string& username, const char* fileName);
    std::string receiveCommand();
    void getFileInfo(const std::string& username, const char* fileName);

private:
    SOCKET clientSocket;
    int sendData(const char* buffer, int len);
    int receiveData(char* buffer, int len);
};