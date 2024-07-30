#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 54000;
const int BUFFER_SIZE = 1024;

std::vector<SOCKET> clients;
std::mutex clientsMutex;

void handle_client(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    while (true) {
        ZeroMemory(buffer, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            closesocket(clientSocket);
            clientsMutex.lock();
            clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
            clientsMutex.unlock();
            break;
        }

        std::string message(buffer, 0, bytesReceived);
        std::cout << "Received: " << message << std::endl;

        clientsMutex.lock();
        for (SOCKET client : clients) {
            if (client != clientSocket) {
                send(client, message.c_str(), message.size() + 1, 0);
            }
        }
        clientsMutex.unlock();
    }
}

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        std::cerr << "Can't start Winsock! " << wsOk << std::endl;
        return -1;
    }

    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));
    listen(listening, SOMAXCONN);

    while (true) {
        SOCKET client = accept(listening, nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            std::cerr << "Client accept failed! " << WSAGetLastError() << std::endl;
            continue;
        }

        clientsMutex.lock();
        clients.push_back(client);
        clientsMutex.unlock();

        std::thread clientThread(handle_client, client);
        clientThread.detach();
    }

    closesocket(listening);
    WSACleanup();
    return 0;
}
