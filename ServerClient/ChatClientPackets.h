#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <string>

namespace user_chat
{

// Типы пакетов
enum PacketType : uint16_t
{
    // от клиента к клиенту
    cpt_undefined = 0,
    cpt_hi,
    cpt_message,
    cpt_status,

    // от сервера к серверу
    spt_already_exists = 100,
    spt_users_list,
    spt_user_status,
};

// Статусы клиента
enum ClientStatus : uint16_t
{
    cst_not_disturb,
    cst_online,
    cst_offline
};

// Пакет "Hi"
struct PacketHi
{
    std::string m_userName;

    PacketHi( std::string userName ) : m_userName(userName) {}

    constexpr static PacketType packetType() { return cpt_hi; }

    template<class ExecutorT>
    void fields( ExecutorT& executor)
    {
        executor(m_userName);
    }
};


// Пакет сообщения
class PacketMessage
{
    std::string m_senderName;
    std::string m_receiverName;
    std::string m_messageText;

public:
    PacketMessage() = default;
    PacketMessage(const std::string& sender, const std::string& receiver, const std::string& message)
        : m_senderName(sender), m_receiverName(receiver), m_messageText(message) {}

    static PacketType packetType() { return cpt_message; }


    // Методы для доступа к данным
    const std::string& getSenderName() const { return m_senderName; }
    const std::string& getReceiverName() const { return m_receiverName; }
    const std::string& getMessageText() const { return m_messageText; }
};

// Пакет статуса клиента
struct PacketClientStatus
{
    std::string     m_myName;
    ClientStatus    m_status;

    PacketClientStatus( std::string myName, ClientStatus status ) : m_myName(myName), m_status(status) {}

    constexpr static PacketType packetType() { return cpt_status; }

    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor( m_myName, (uint16_t&)m_status );
    }

    // Методы для доступа к данным
    const std::string& getMyName() const { return m_myName; }
    ClientStatus getStatus() const { return m_status; }
};

// Пакет "Пользователь уже существует"
struct ServerPacketUserAlreadyExists
{
    constexpr static PacketType packetType() { return spt_already_exists; }

    template<class ExecutorT>
    void fields( const ExecutorT& executor ) {}
};

struct UserStatus
{
public:
    std::string     m_playerName;
    ClientStatus    m_status = user_chat::cst_not_disturb;

    template<class ExecutorT>
    void fields( ExecutorT& executor )
    {
        executor( m_playerName, reinterpret_cast<uint16_t&>( m_status ) );
    }

    // template<class ExecutorT>
    // void fields( ExecutorT& executor ) const
    // {
    //     executor( m_playerName, reinterpret_cast<const uint16_t&>( m_status ) );
    // }
};


// Пакет списка пользователей
struct ServerPacketUsersList
{
    std::vector<UserStatus> m_usersList;

    ServerPacketUsersList() = default;
    ServerPacketUsersList(std::vector<UserStatus>&& usersList) : m_usersList(std::move(usersList)) {}

    PacketType packetType() const { return spt_users_list; }

    template<class ExecutorT>
        void fields( ExecutorT& executor )
    {
        executor( m_usersList );
    }

    const std::vector<UserStatus>& getUsersList() const { return m_usersList; }
};

}

