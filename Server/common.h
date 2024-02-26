#pragma once
#include <iostream>
#include <WinSock2.h>
#include <string>
#include <fstream>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

enum Commands {
    MESSAGE_ = 1,
    FILE_ = 2,
    EXIT_ = 3,
    YES_ = 4,
    NO_ = 5
};

class CommonCode {
public:
    CommonCode(SOCKET clientSocket, const string& clientFolder);

    void sendChunkedData(const char* data, int dataSize, int chunkSize) const;
    string receiveChunkedData() const;
    void sendFile(const string& filename, const string& senderFolder) const;
    void insertFile(const string& filename) const;
private:
    SOCKET s;
    const string& clientFolder;
};