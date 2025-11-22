// server.cpp - Windows (Winsock) simple broadcast server
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>

#pragma comment(lib, "Ws2_32.lib")

constexpr int PORT = 54000;
std::vector<SOCKET> clients;
std::mutex clients_mtx;

void broadcast(const std::string& msg, SOCKET except = INVALID_SOCKET) {
    std::lock_guard<std::mutex> lk(clients_mtx);
    for (SOCKET s : clients) {
        if (s == except) continue;
        int result = send(s, msg.c_str(), (int)msg.size(), 0);
        if (result == SOCKET_ERROR) {
            int err = WSAGetLastError();
            std::cerr << "Send error to client (error: " << err << "), may need to disconnect\n";
        }
    }
}

void handle_client(SOCKET client) {
    char buf[1024];
    std::cout << "Client connected\n";
    
    while (true) {
        int n = recv(client, buf, (int)sizeof(buf) - 1, 0);
        if (n <= 0) {
            if (n == 0) {
                std::cout << "Client gracefully disconnected\n";
            } else {
                int err = WSAGetLastError();
                std::cerr << "recv() error: " << err << "\n";
            }
            break;
        }
        buf[n] = '\0';
        std::string msg = std::string(buf);
        std::cout << "Broadcasting: " << msg;
        broadcast(msg, client);
    }

    closesocket(client);
    {
        std::lock_guard<std::mutex> lk(clients_mtx);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }
    std::cout << "Client removed. Active clients: " << clients.size() << "\n";
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_sock == INVALID_SOCKET) {
        std::cerr << "socket() failed\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    if (bind(listen_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind() failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() failed\n";
        closesocket(listen_sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "\n";

    while (true) {
        SOCKET client = accept(listen_sock, nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            std::cerr << "accept() failed\n";
            break;
        }
        {
            std::lock_guard<std::mutex> lk(clients_mtx);
            clients.push_back(client);
        }
        std::cout << "New client connected. Total clients: " << clients.size() << "\n";
        std::thread(handle_client, client).detach();
    }

    closesocket(listen_sock);
    WSACleanup();
    return 0;
}
