#include "server.h"

mutex m;
mutex queueM;
condition_variable cv;
queue<Data> dataQueue;

Server::Server() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    rooms["room1"] = map<SOCKET, pair<string, string>>();
    rooms["room2"] = map<SOCKET, pair<string, string>>();
    rooms["room3"] = map<SOCKET, pair<string, string>>();
}

void Server::start() {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
    bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);
    cout << "Server is waiting for incoming connections on port " << port << "..." << endl;
    thread queueThread(&Server::processQueue, this);
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        lock_guard<mutex> lock(m);
        clientCounter++;
        if (clientCounter == 6) {
            for (auto& room : rooms) {
                for (auto& user : room.second) {
                    closesocket(user.first);
                }
            }
            cout << "Server is full, closing all connections" << endl;
            break;
        }
        thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }
    queueThread.join();
    closesocket(serverSocket);
    WSACleanup();
}

void Server::handleClient(SOCKET clientSocket) {
    pair<string, int> clientData = getClientData(clientSocket);
    string clientFolder = clientData.first;
    int clientNumber = clientData.second;
    CommonCode commonCode(clientSocket, clientFolder);
    while (true) {
        string lowerRoomSelection;
        commonCode.sendChunkedData(roomData().c_str(), roomData().size(), 10);
        string roomSelection = commonCode.receiveChunkedData();
        transform(roomSelection.begin(), roomSelection.end(), back_inserter(lowerRoomSelection), ::tolower);
        if (lowerRoomSelection == "exit") {
            cout << "Client" << clientNumber << " has disconnected" << endl;
            lock_guard<mutex> lock(m);
            clientCounter--;
            closesocket(clientSocket);
            return;
        }
        if (rooms.find(lowerRoomSelection) != rooms.end()) {
            rooms[roomSelection].insert({clientSocket, {clientFolder, "Client" + to_string(clientNumber)}});
            cout << "Client" << clientNumber << " has joined " << roomSelection << endl;
            commonCode.sendChunkedData(("You have joined " + roomSelection).c_str(), strlen(("You have joined " + roomSelection).c_str()), 10);
            processCommands(commonCode, roomSelection, clientSocket);
        } else {
            commonCode.sendChunkedData("Invalid room", strlen("Invalid room"), 10);
        }
    }
}

void Server::processCommands(CommonCode& commonCode, const string& roomSelection, SOCKET clientSocket) {
    while (true) {
        int command;
        recv(clientSocket, reinterpret_cast<char*>(&command), sizeof(int), 0);
        if (command == MESSAGE_) {
            addToQueue(commonCode, roomSelection, clientSocket, MESSAGE_);
        } else if (command == FILE_) {
            addToQueue(commonCode, roomSelection, clientSocket, FILE_);
        } else if (command == YES_) {
            addToQueue(commonCode, roomSelection, clientSocket, YES_);
        } else if (command == NO_) {
            addToQueue(commonCode, roomSelection, clientSocket, NO_);
        } else {
            cout << rooms[roomSelection][clientSocket].second << " has left " << roomSelection << endl;
            rooms[roomSelection].erase(clientSocket);
            return;
        }
    }
}

void Server::addToQueue(CommonCode& commonCode, const string& roomSelection, SOCKET clientSocket, int command) {
    string prompt = commonCode.receiveChunkedData();
    Data data = {roomSelection, clientSocket, command, prompt};
    {
        lock_guard<mutex> lock(queueM);
        dataQueue.push(data);
    }
    cv.notify_one();
}

void Server::processQueue() {
    while (true) {
        unique_lock<mutex> lock(queueM);
        cv.wait(lock, [] { return !dataQueue.empty(); });
        while (!dataQueue.empty()) {
            Data data = dataQueue.front();
            dataQueue.pop();
            processEntry(data.room, data.clientS, data.command, data.prompt);
        }
    }
}

void Server::processEntry(const string& room, SOCKET clientSocket, int command, const string& prompt) {
    for (auto& user : rooms[room]) {
        CommonCode commonCode(user.first, user.second.first);
        if (user.first != clientSocket) {
            if (command == MESSAGE_) {
                string message = rooms[room][clientSocket].second + ": " + prompt;
                commonCode.sendChunkedData(message.c_str(), message.size(), 10);
            } else if (command == FILE_) {
                string filePrompt = rooms[room][clientSocket].second + " wants to send " + prompt + " file, do you want to receive?";
                dataToSend[room] = {rooms[room][clientSocket].first, prompt};
                cout << rooms[room][clientSocket].second << " wants to send file" << endl;
                commonCode.sendChunkedData(filePrompt.c_str(), filePrompt.size(), 10);
            }
        } else {
            if (command == YES_) {
                string senderFolder = dataToSend[room].first;
                string filename = dataToSend[room].second;
                cout << user.second.second << " has accepted the file transfer" << endl;
                commonCode.sendFile(filename, senderFolder);
            } else if (command == NO_) {
                cout << user.second.second << " has rejected the file transfer" << endl;
            }
        }
    }
}

string Server::roomData() {
    string roomsData = "Select any room:\n";
    for (auto& room : rooms) {
        roomsData += room.first + ": ";
        if (room.second.empty()) {
            roomsData += "empty :(";
        } else {
            vector<string> usersInRoom;
            for (auto& user : room.second) {
                usersInRoom.push_back(user.second.second);
            }
            roomsData += "[";
            for (size_t i = 0; i < usersInRoom.size(); ++i) {
                roomsData += usersInRoom[i];
                if (i != usersInRoom.size() - 1) {
                    roomsData += ", ";
                }
            }
            roomsData += "]";
        }
        if (room.first != rooms.rbegin()->first) {
            roomsData += "\n";
        }
    }
    return roomsData;
}

pair<string, int> Server::getClientData(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    buffer[bytesReceived] = '\0';
    string path(buffer);
    size_t lastSlashPos = path.find_last_of("/\\");
    string clientFolder = path.substr(lastSlashPos + 1);
    int clientNumber = stoi(clientFolder.substr(6));
    cout << "Client" << clientNumber << " is connected" << endl;
    return {path, clientNumber};
}