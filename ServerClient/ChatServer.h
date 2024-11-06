#pragma once
#include "TcpServer.h"
#include <map>
#include <memory>

class ChatSession : public TcpClientSession {
public:
    ChatSession(boost::asio::ip::tcp::socket socket)
        : TcpClientSession(std::move(socket)) {}

    void start() {
        readPacketHeader(); // Start reading packets
    }
};

class ChatServer : public TcpServer {
public:
    ChatServer(const std::string& addr, const std::string& port)
        : TcpServer(addr, port) {}

    void onClientConnected(std::shared_ptr<ChatSession> session) {
        int clientId = ++m_nextClientId;
        m_sessions[clientId] = session;
        LOG("Client " << clientId << " connected");

        // Начинаем чтение данных от клиента
        session->start();
    }

    void onClientDisconnected(int clientId) {
        LOG("Client " << clientId << " disconnected");
        m_sessions.erase(clientId);
    }

    void onPacketReceived(int clientId, const std::vector<uint8_t>& packetData) {
        try {
            auto session = m_sessions.find(clientId);
            if (session != m_sessions.end()) {
                session->second->onPacketReceived(packetData);
            } else {
                LOG("Error: Client " << clientId << " not found");
            }
        } catch (const std::exception& e) {
            LOG("Exception while processing packet: " << e.what());
        }
    }

private:
    int m_nextClientId = 0;
    std::map<int, std::shared_ptr<ChatSession>> m_sessions;
};

