#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

const int PORT = 54000;
const int BUFFER_SIZE = 1024;

int main() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        std::cerr << "Can't start Winsock! " << wsOk << std::endl;
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Can't create a socket! Quitting" << std::endl;
        return -1;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Can't connect to server! " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    char buffer[BUFFER_SIZE];
    std::string userInput;
    while (true) {
        std::getline(std::cin, userInput);
        if (userInput == "exit") break;

        int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult != SOCKET_ERROR) {
            ZeroMemory(buffer, BUFFER_SIZE);
            int bytesReceived = recv(sock, buffer, BUFFER_SIZE, 0);
            if (bytesReceived > 0) {
                std::cout << "SERVER> " << std::string(buffer, 0, bytesReceived) << std::endl;
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
