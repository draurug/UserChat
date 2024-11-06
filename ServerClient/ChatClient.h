#pragma once
#include "ChatClientPackets.h"
#include "ChatClientPacketUtils.h"
#include "TcpClient.h"

namespace user_chat
{

class ChatClient : public TcpClient
{
    std::string m_userName;

public:
    ChatClient(const std::string& userName) : m_userName(userName) {}

    template <class PacketT>
    void sendPacket(PacketT& packet)
    {
        //create TCP packet (Envelope) лучше отказаться
        PacketSizeCalculator sizeCalculator;
        sizeCalculator.addSize(uint16_t{});  // Размер TCP пакета
        sizeCalculator.addSize(uint16_t{});  // Тип пакета
        sizeCalculator.addSize(packet);      // Поля пакета

        size_t packetSize = sizeCalculator.getSize();
        uint8_t* buffer = new uint8_t[packetSize];
        PacketWriter writer(buffer, packetSize);

        writer.write(static_cast<uint16_t>(packetSize));
        writer.write(static_cast<uint16_t>(PacketType()));
        writer.write(packet);

        TcpClient::sendPacket(buffer, packetSize);
    }

    void onConnected(const boost::system::error_code& ec) override
    {
        if (!ec)
        {
            LOG("Successfully connected to the server!");
            PacketHi packet{m_userName};
            sendPacket(packet);
        } else
        {
            LOG_ERR("Error connecting: " << ec.message());
        }
    }

    void onPacketReceived(const uint8_t* data, size_t dataSize)
    {
        user_chat::PacketReader reader(data, data + dataSize);

        std::string userName;
        reader.read(userName);
        if (userName.empty()) {
            PacketType packetType;
            reader.read(reinterpret_cast<uint16_t&>(packetType));

            switch (packetType) {
            case spt_users_list: {
                ServerPacketUsersList packet;
                reader.read(packet);
                // Вызываем метод обработки списка пользователей
               onUsersListReceived(packet.m_usersList);
                break;
            }
                // Добаввить обработку других типов пакетов -? предлагает qtCreator
            case cpt_undefined:
            case cpt_hi:
            case cpt_message:
            case cpt_status:
            case spt_already_exists:
            case spt_user_status:
                break;
            }
        } else {
            PacketType packetType;
            reader.read(reinterpret_cast<uint16_t&>(packetType));
        }
    }

    void onUsersListReceived(std::vector<UserStatus>& m_usersList)
    {
        // Обработка списка пользователей
    }


};
}

