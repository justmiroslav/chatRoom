#include "client.h"

Client::Client() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
}

void Client::start() {
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(1337);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(clientSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    cout << "Connected to server." << endl;
    send(clientSocket, clientFolder.c_str(), clientFolder.size(), 0);
    CommonCode commonCode(clientSocket, clientFolder);
    while (true) {
        string roomSelection, lowerRoomSelection;
        if (!isRoomSelected) {
            string roomData = commonCode.receiveChunkedData();
            cout << roomData << endl;
        }
        getline(cin, roomSelection);
        transform(roomSelection.begin(), roomSelection.end(), back_inserter(lowerRoomSelection), ::tolower);
        commonCode.sendChunkedData(lowerRoomSelection.c_str(), lowerRoomSelection.size(), 10);
        string serverResponse = commonCode.receiveChunkedData();
        if (lowerRoomSelection == "exit") {
            cout << "Exiting chat..." << endl;
            break;
        }
        cout << serverResponse << endl;
        if (serverResponse != "Invalid room") {
            isRunning = true;
            thread receiveThread(&Client::receiveData, this, ref(commonCode));
            sendData(commonCode);
            isRoomSelected = true;
            cout << "Exiting " + roomSelection + "..." << endl;
            receiveThread.join();
        }
    }
    closesocket(clientSocket);
    WSACleanup();
}

void Client::sendData(CommonCode& commonCode) {
    while (isRunning) {
        int command;
        cout << "Enter command (1-MESSAGE/2-FILE/3-EXIT/4-YES/5-NO):" << endl;
        cin >> command;
        cin.ignore();
        send(clientSocket, reinterpret_cast<const char*>(&command), sizeof(int), 0);
        if (command == EXIT_) {
            isRunning = false;
            break;
        }
        if (command == MESSAGE_ || command == FILE_) {
            string input;
            string prompt = (command == MESSAGE_) ? "Enter message:" : "Enter filename:";
            cout << prompt << endl;
            getline(cin, input);
            commonCode.sendChunkedData(input.c_str(), input.size(), 10);
        } else if (command == YES_) {
            commonCode.sendChunkedData("yes", strlen("yes"), 10);
            isFile = true;
        } else if (command == NO_) {
            commonCode.sendChunkedData("no", strlen("no"), 10);
            isFile = false;
        } else {
            cout << "Invalid command" << endl;
        }
    }
}

void Client::receiveData(CommonCode& commonCode) {
    while (isRunning) {
        string message = commonCode.receiveChunkedData();
        if (!isFile) {
            lock_guard<mutex> lock(m);
            cout << message << endl;
        } else {
            commonCode.insertFile(message);
            isFile = false;
        }
    }
}