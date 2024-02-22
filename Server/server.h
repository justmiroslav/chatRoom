#pragma once
#include <thread>
#include <mutex>
#include <map>
#include <queue>
#include <condition_variable>
#include <algorithm>
#include <cctype>
#include "common.h"

struct Data {
    string room;
    SOCKET clientS;
    int command;
    string prompt;
};

class Server {
public:
    Server();
    void start();
    void handleClient(SOCKET clientSocket);
    void processCommands(CommonCode& commonCode, const string& roomSelection, SOCKET clientSocket);
    static void addToQueue(CommonCode& commonCode, const string& roomSelection, SOCKET clientSocket, int command);
    void processQueue();
    void processEntry(const string& room, SOCKET clientSocket, int command, const string& prompt);

private:
    SOCKET serverSocket;
    sockaddr_in serverAddr;
    map<string, map<SOCKET, pair<string, string>>> rooms;
    int port = 1337;
    int clientCounter = 0;

    string roomData();
    static pair<string, int> getClientData(SOCKET clientSocket);
};