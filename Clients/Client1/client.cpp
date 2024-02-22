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
        string roomData = commonCode.receiveChunkedData();
        cout << roomData << endl;
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
            receiveThread.join();
            cout << "Exiting " + roomSelection + "..." << endl;
        }
    }
    closesocket(clientSocket);
    WSACleanup();
}

void Client::sendData(CommonCode& commonCode) {
    while (true) {
        int command;
        cout << "Enter command (1-MESSAGE/2-FILE/3-EXIT):" << endl;
        cin >> command;
        cin.ignore();
        if (command == EXIT_ || command == MESSAGE_ || command == FILE_) {
            send(clientSocket, reinterpret_cast<const char*>(&command), sizeof(int), 0);
            if (command == EXIT_) {
                isRunning = false;
                break;
            }
            string input;
            string prompt = (command == MESSAGE_) ? "Enter message:" : "Enter filename:";
            cout << prompt << endl;
            getline(cin, input);
            commonCode.sendChunkedData(input.c_str(), input.size(), 10);
        } else {
            cout << "Invalid command" << endl;
        }
    }
}

void Client::receiveData(CommonCode& commonCode) {
    while (isRunning) {
        int command;
        string input, lowerInput;
        recv(clientSocket, reinterpret_cast<char *>(&command), sizeof(int), 0);
        string message = commonCode.receiveChunkedData();
        cout << message << endl;
        if (command == FILE_) {
            getline(cin, input);
            commonCode.sendChunkedData(input.c_str(), input.size(), 10);
            transform(input.begin(), input.end(), back_inserter(lowerInput), ::tolower);
            if (lowerInput == "yes") {
                commonCode.insertFile();
            }
        }
    }
}