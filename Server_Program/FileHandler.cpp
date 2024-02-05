#include "FileHandler.h"
std::mutex m;

FileHandler::FileHandler(SOCKET clientSocket) : clientSocket(clientSocket) {}

int FileHandler::sendData(const char* buffer, int len) {
    return send(clientSocket, buffer, len, 0);
}

int FileHandler::receiveData(char* buffer, int len) {
    return recv(clientSocket, buffer, len, 0);
}

void FileHandler::receiveFile(const std::string& username) {
    char buffer[2048];
    std::string directoryPath = username + "/";
    int fileNameLength;
    int bytesReceived = receiveData(reinterpret_cast<char*>(&fileNameLength), sizeof(int));
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        std::cout << "Error in receiving filename length." << std::endl;
        return;
    }

    std::vector<char> fileNameBuffer(fileNameLength);
    bytesReceived = receiveData(fileNameBuffer.data(), fileNameLength);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        std::cout << "Error in receiving filename." << std::endl;
        return;
    }

    std::string fileName(fileNameBuffer.begin(), fileNameBuffer.end());
    std::string fullPath = directoryPath + fileName;
    std::ofstream outputFile(fullPath, std::ios::binary);

    int totalSize;
    bytesReceived = receiveData(reinterpret_cast<char*>(&totalSize), sizeof(int));
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        std::cout << "Error in receiving total size." << std::endl;
        return;
    }

    int totalReceived = 0;
    while (totalReceived < totalSize) {
        bytesReceived = receiveData(buffer, sizeof(buffer));
        if (bytesReceived > 0) {
            outputFile.write(buffer, bytesReceived);
            totalReceived += bytesReceived;
        }
        else {
            break;
        }
    }
    outputFile.close();
}

void FileHandler::sendFile(const std::string& username, const char* fileName) {
    char buffer[1024];
    std::string directoryPath = username + "/";
    std::string fullPath = directoryPath + fileName;
    std::ifstream inputFile(fullPath.c_str(), std::ios::binary);
    inputFile.seekg(0, std::ios::end);
    int totalSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    sendData(reinterpret_cast<char*>(&totalSize), sizeof(int));
    int totalBytesSent = 0;
    while (inputFile) {
        inputFile.read(buffer, sizeof(buffer));
        int bytesRead = inputFile.gcount();
        if (bytesRead > 0) {
            sendData(buffer, bytesRead);
            totalBytesSent += bytesRead;
            std::cout << "Sent " << bytesRead << " bytes, total: " << totalBytesSent << " bytes" << std::endl;
        }
    }
    inputFile.close();
}

void FileHandler::listFilesInDirectory(const std::string& username) {
    WIN32_FIND_DATAA fileData;
    HANDLE hFind;
    std::cout << "List of files in the client's directory" << std::endl;
    std::string directoryPath = username + "\\*";
    if (!((hFind = FindFirstFileA(directoryPath.c_str(), &fileData)) == INVALID_HANDLE_VALUE)) {
        do {
            if (!(fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string fileName = fileData.cFileName;
                fileName += "\n";
                sendData(fileName.c_str(), fileName.size() + 1);
                std::cout << fileName;
            }
        } while (FindNextFileA(hFind, &fileData));
        FindClose(hFind);
    }
    else {
        std::cout << "Path not found" << std::endl;
    }
    std::cout << "-----------------------" << std::endl;
    std::string endMarker = "END";
    sendData(endMarker.c_str(), endMarker.size() + 1);
}

void FileHandler::deleteFile(const std::string& username, const char* fileName) {
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

std::string FileHandler::receiveCommand() {
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

void FileHandler::getFileInfo(const std::string& username, const char* fileName) {
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

        std::string timeInfo = "Last modified: " + std::to_string(stLocal.wDay) + "/" +
            std::to_string(stLocal.wMonth) + "/" +
            std::to_string(stLocal.wYear) + " " +
            std::to_string(stLocal.wHour) + ":" +
            std::to_string(stLocal.wMinute) + "\n";
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
