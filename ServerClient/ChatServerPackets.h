#pragma once

#include "ChatClientPackets.h"

namespace user_chat {

// Базовый класс для всех пакетов от сервера
class ServerPacketBase
{
    virtual PacketType packetType() const = 0;
    virtual void serialiseFields() {};
};

// Пакет со статусом пользователя
class ServerPacketUserStatus : public ServerPacketBase {
    std::string m_userName;
    ClientStatus m_status;

public:
    ServerPacketUserStatus(const std::string& userName, ClientStatus status)
        : m_userName(userName), m_status(status) {}

    PacketType packetType() const override { return PacketType::spt_user_status; }

    void serialiseFields() override {
        std::cout << "Serializing ServerPacketUserStatus: " << m_userName
                  << ", Status: " << m_status << std::endl;
    }

    const std::string& getUserName() const { return m_userName; }
    ClientStatus getStatus() const { return m_status; }
};

}

