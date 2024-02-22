#pragma once
#include "common.h"
#include <thread>
#include <cctype>
#include <algorithm>
#include <atomic>

class Client {
public:
    Client();
    void start();
    void sendData(CommonCode& commonCode);
    void receiveData(CommonCode& commonCode);

private:
    atomic<bool> isRunning;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    const string clientFolder = "C:/Users/Admin/CLionProjects/ChatRoom/Clients/Client1";
};