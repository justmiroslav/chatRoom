#include "common.h"

CommonCode::CommonCode(SOCKET clientSocket, const string& clientFolder) : s(clientSocket), clientFolder(clientFolder) {}

void CommonCode::sendChunkedData(const char* data, int dataSize, int chunkSize) const {
    int totalSent = 0;
    send(s, reinterpret_cast<const char*>(&dataSize), sizeof(int), 0);
    while (totalSent < dataSize) {
        int remaining = dataSize - totalSent;
        int currentChunkSize = (remaining < chunkSize) ? remaining : chunkSize;
        send(s, reinterpret_cast<const char*>(&currentChunkSize), sizeof(int), 0);
        send(s, data + totalSent, currentChunkSize, 0);
        totalSent += currentChunkSize;
    }
}

string CommonCode::receiveChunkedData() const {
    int chunkSize, totalSize = 0;
    string data;
    recv(s, reinterpret_cast<char*>(&totalSize), sizeof(int), 0);
    do {
        recv(s, reinterpret_cast<char*>(&chunkSize), sizeof(int), 0);
        if (chunkSize > 0) {
            char* buffer = new char[chunkSize + 1];
            recv(s, buffer, chunkSize, 0);
            data.append(buffer, chunkSize);
            delete[] buffer;
        }
    } while (data.size() < totalSize);
    return data;
}

void CommonCode::sendFile(const string& filename, const string& senderFolder) const {
    sendChunkedData(filename.c_str(), filename.size(), 10);
    ifstream file(senderFolder + "/" + filename);
    if (!file.is_open()) {
        sendChunkedData("File not found", strlen("File not found"), 10);
        return;
    }
    string data;
    char character;
    while (file.get(character)) {
        data += character;
    }
    file.close();
    sendChunkedData(data.c_str(), data.size(), 1024);
}

void CommonCode::insertFile(const string& filename) const {
    string data = receiveChunkedData();
    if (data == "File not found") {
        return;
    }
    ofstream file(clientFolder + "/" + filename);
    file << data;
    file.close();
}
