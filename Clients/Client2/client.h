#pragma once
#include "common.h"
#include <thread>
#include <cctype>
#include <algorithm>
#include <atomic>
#include <mutex>

class Client {
public:
    Client();
    void start();
    void sendData(CommonCode& commonCode);
    void receiveData(CommonCode& commonCode);

private:
    mutex m;
    atomic<bool> isRunning;
    atomic<bool> isFile = false;
    atomic<bool> isRoomSelected = false;
    SOCKET clientSocket;
    sockaddr_in serverAddr;
    const string clientFolder = "C:/Users/miros/CLionProjects/chatRoom/Clients/Client2";
};