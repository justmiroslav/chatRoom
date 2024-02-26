# ChatRoom Documentation

## 1. General system description

#### *This project is a chat application developed in C++. It consists of a server and multiple clients. The server manages the connections and the chat rooms, while the clients send and receive messages. The system uses the WinSock2 library for network communication and is built using a multi-threaded architecture, where each client connection is handled in a separate thread. This allows the server to handle multiple clients simultaneously.*

## 2. Application protocol description

+ **﻿﻿Connection Establishment:** The client initializes the Winsock library, creates a socket, and connects to the server using the ```connect``` function. The server initializes the Winsock library, creates a socket, binds it to a specific port, and starts listening for incoming connections. When a connection request is received, the server accepts it and creates a new thread to handle the client.
+ **Room Selection:** After the connection is established, the client can select a chat room. The server sends the list of available chat rooms to the client. The client sends the selected room name to the server. If the room is valid, the client is added to the room, otherwise, an error message is sent back to the client.
+ **Data Exchange:** Once inside a room, the client can send messages or files to other clients in the same room, accept and decline file insertion and leaving the room.
  + ***Message transferring:*** Client enters the message and sends it to the server with the following command. The server receives this command and forwards the message to all other clients in the room.
  + ***File transferring:*** Client enters the filename and sends it to the server with the following command. The server asks all other clients in the room if they want to receive this file.
  + ***Accepting/Declining file insertion:*** Clients sends an appropriate command to the server indicating that if they want to insert a file or not. The file is then sent from the server to the clients who accepted the file insertion.
  + ***Room leaving:*** Client sends an appropriate command to the server, which then removes the client from the room.
+ **Client Disconnection:** While selecting room to join client enter ```"exit"``` to leave the chat. The server then closes the connection with the client and cleans up any resources associated with the client.
+ **Commands and Their Usage:** The client can send the following commands to the server:
  + ```MESSAGE_```: Command for sending message to the room.
  + ```FILE_```: Command for sending file to the room.
  + ```YES_```: Command for accepting file insertion.
  + ```NO_```: Command for declining file insertion.
  + ```EXIT_```: Command for leaving the room.

The application uses the TCP protocol, which ensures data integrity. About how data is transferred through the network check Topic 3 [here](https://github.com/justmiroslav/Client-Server?tab=readme-ov-file#3---data-transfer)

## 3. Screenshots and code examples of different use cases

### Join Room
Client:

![client room select](https://github.com/justmiroslav/chatRoom/assets/119401743/10535e8b-2e93-44da-87be-4b61b021fd29)
```c++
// void Client::start()
string roomSelection, lowerRoomSelection;
if (!isRoomSelected) {
    string roomData = commonCode.receiveChunkedData();
    cout << roomData << endl;
}
getline(cin, roomSelection);
transform(roomSelection.begin(), roomSelection.end(), back_inserter(lowerRoomSelection), ::tolower);
commonCode.sendChunkedData(lowerRoomSelection.c_str(), lowerRoomSelection.size(), 10);
string serverResponse = commonCode.receiveChunkedData();
//
cout << serverResponse << endl;
//
```

Server:

![server room select](https://github.com/justmiroslav/chatRoom/assets/119401743/0bcd773d-55a5-4a6f-b870-7529c472831b)
```c++
//void Server::handleClient(SOCKET clientSocket)
string lowerRoomSelection;
commonCode.sendChunkedData(roomData().c_str(), roomData().size(), 10);
string roomSelection = commonCode.receiveChunkedData();
transform(roomSelection.begin(), roomSelection.end(), back_inserter(lowerRoomSelection), ::tolower);
//
if (rooms.find(lowerRoomSelection) != rooms.end()) {
    rooms[roomSelection].insert({clientSocket, {clientFolder, "Client" + to_string(clientNumber)}});
    cout << "Client" << clientNumber << " has joined " << roomSelection << endl;
    commonCode.sendChunkedData(("You have joined " + roomSelection).c_str(), strlen(("You have joined " + roomSelection).c_str()), 10);
    processCommands(commonCode, roomSelection, clientSocket);
} //
```

### Exit Room
Client:

![client room leaving](https://github.com/justmiroslav/chatRoom/assets/119401743/78ee0e1d-9225-45f4-8060-654b02b6d26b)
```c++
//void Client::start()
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
    //
```

Server:

![server room leaving](https://github.com/justmiroslav/chatRoom/assets/119401743/5016bcf6-16ce-4c4b-beed-be912a3f5109)
```c++
//void Server::processCommands(CommonCode& commonCode, const string& roomSelection, SOCKET clientSocket)
while (true) {
    //
    else {
        cout << rooms[roomSelection][clientSocket].second << " has left " << roomSelection << endl;
        rooms[roomSelection].erase(clientSocket);
        return;
    }
//
```

### Leave Chat
Client:

![client chat leaving](https://github.com/justmiroslav/chatRoom/assets/119401743/3a5011e1-d2af-4e06-8928-ee13232dc2d7)
```c++
//void Client::start()
//
if (lowerRoomSelection == "exit") {
    cout << "Exiting chat..." << endl;
    break;
}
//
```

Server:

![server chat leaving](https://github.com/justmiroslav/chatRoom/assets/119401743/9e400cbe-4d55-4a37-9f52-5e6fb76a6260)
```c++
//void Server::handleClient(SOCKET clientSocket)
//
if (lowerRoomSelection == "exit") {
    cout << "Client" << clientNumber << " has disconnected" << endl;
    lock_guard<mutex> lock(m);
    clientCounter--;
    closesocket(clientSocket);
    return;
}
//
```

### Send Message
Client1:

![client1 message](https://github.com/justmiroslav/chatRoom/assets/119401743/6df6a55a-5ec2-42b7-83d8-fd499b436d08)
```c++
//void Client::receiveData(CommonCode& commonCode)
while (isRunning) {
    string message = commonCode.receiveChunkedData();
    if (!isFile) {
        lock_guard<mutex> lock(m);
        cout << message << endl;
    } //
```

Client2:

![client2 message](https://github.com/justmiroslav/chatRoom/assets/119401743/f7da52e3-4d74-41ed-af9f-7dcb16e9132a)
```c++
//void Client::sendData(CommonCode& commonCode)
if (command == MESSAGE_ || command == FILE_) {
    string input;
    string prompt = (command == MESSAGE_) ? "Enter message:" : "Enter filename:";
    cout << prompt << endl;
    getline(cin, input);
    commonCode.sendChunkedData(input.c_str(), input.size(), 10);
} //
```

### Send, Accept and Decline File
Client1:

![client1 file](https://github.com/justmiroslav/chatRoom/assets/119401743/4805c5a6-e4ad-49ab-9f07-ad668882dc8a)
```c++
//void Client::sendData(CommonCode& commonCode)
//
} else if (command == NO_) {
    commonCode.sendChunkedData("no", strlen("no"), 10);
    isFile = false;
}
//
```

Client2:

![client2 file](https://github.com/justmiroslav/chatRoom/assets/119401743/e9e60771-77c4-4398-9943-6cf9fd98dc90)
```c++
//void Client::sendData(CommonCode& commonCode)
//
} else if (command == YES_) {
    commonCode.sendChunkedData("yes", strlen("yes"), 10);
    isFile = true;
} //
//void Client::receiveData(CommonCode& commonCode)
//
else {
        commonCode.insertFile(message);
        isFile = false;
    }
// 
```

Client1 Folder:

![client1 folder](https://github.com/justmiroslav/chatRoom/assets/119401743/49b7170b-48b6-40b1-b110-bfa5217cd48b)

Client2 Folder:

![client2 folder](https://github.com/justmiroslav/chatRoom/assets/119401743/2b8d9255-f62e-49ec-b9fc-f76341f6d521)

### Change Room
Client1:

![client1 change room](https://github.com/justmiroslav/chatRoom/assets/119401743/cc45ed32-7928-4b49-a299-b0101d8bdbe3)

Client2:

![client2 change room](https://github.com/justmiroslav/chatRoom/assets/119401743/d168d2dd-d459-4717-a8e8-25db32ccf6bb)
```c++
//void Client::start()
while (true) {
    //
    if (serverResponse != "Invalid room") {
        isRunning = true;
        thread receiveThread(&Client::receiveData, this, ref(commonCode));
        sendData(commonCode);
        isRoomSelected = true;
        cout << "Exiting " + roomSelection + "..." << endl;
        receiveThread.join();
    }
}
```
