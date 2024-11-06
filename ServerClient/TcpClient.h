#pragma once
#include "ChatClientPackets.h"
#include "Logs.h"
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <vector>

using boost::asio::ip::tcp;

class IAppliedTcpClient
{
public:
    virtual ~IAppliedTcpClient() = default;
    virtual void onConnected(const boost::system::error_code& ec) = 0;
    virtual void sendPacketTo(user_chat::PacketBase& packet) = 0;

};

class TcpClient : public std::enable_shared_from_this<TcpClient>, public IAppliedTcpClient
{
    boost::asio::io_context m_context;
    tcp::resolver m_resolver;
    tcp::socket m_socket;
    uint16_t m_dataLength;
    std::vector<uint8_t> m_packetData;

public:
    TcpClient()
        : m_context(), m_resolver(m_context), m_socket(m_context), m_dataLength(0)
    {
    }

    void run(const std::string& host, const std::string& port)
    {
        tcp::resolver::query query(host, port);
        m_resolver.async_resolve(query,
                                 [self = shared_from_this()](const boost::system::error_code& ec, tcp::resolver::iterator endpoint_iterator) {
                                     self->onResolve(ec, endpoint_iterator);
                                 });

        m_context.run();
    }

protected:
    void onResolve(const boost::system::error_code& ec, tcp::resolver::iterator endpoint_iterator)
    {
        if (!ec)
        {
            boost::asio::async_connect(m_socket, endpoint_iterator,
                                       [self = shared_from_this()](const boost::system::error_code& ec, tcp::resolver::iterator) {
                                           self->onConnect(ec);
                                       });
        }
        else
        {
            LOG_ERR("Error resolving: " << ec.message());
        }
    }

    void onConnect(const boost::system::error_code& ec)
    {
        if (!ec)
        {
            LOG("Successfully connected to the server!");
            onConnected(ec);
            readPacketHeader();
        }
        else
        {
            LOG_ERR("Error connecting: " << ec.message());
        }
    }

    void readPacketHeader()
    {
        boost::asio::async_read(m_socket, boost::asio::buffer(&m_dataLength, sizeof(m_dataLength)),
                                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                                    self->onReadPacketHeader(error, bytes_transferred);
                                });
    }

    void onReadPacketHeader(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (error)
        {
            LOG_ERR("TcpClient read error: " << error.message());
            return;
        }
        if (bytes_transferred != sizeof(m_dataLength))
        {
            LOG_ERR("TcpClient read error (m_dataLength): " << bytes_transferred << " vs " << sizeof(m_dataLength));
            return;
        }

        LOG("TcpClient received: " << m_dataLength);

        if (m_dataLength == 0 || m_dataLength > 16 * 1024)
        {
            LOG_ERR("TcpClient invalid dataLength: " << m_dataLength);
            return;
        }

        m_packetData.resize(m_dataLength);
        boost::asio::async_read(m_socket, boost::asio::buffer(m_packetData.data(), m_dataLength),
                                [self = shared_from_this()](const boost::system::error_code& error, size_t bytes_transferred) {
                                    self->readPacketData(error, bytes_transferred);
                                });
    }

    void readPacketData(const boost::system::error_code& error, size_t bytes_transferred)
    {
        if (error)
        {
            LOG_ERR("TcpClient read error: " << error.message());
            return;
        }
        if (bytes_transferred != m_dataLength)
        {
            LOG_ERR("TcpClient read error (bytes_transferred): " << bytes_transferred << " vs " << m_dataLength);
            return;
        }

        // Обработка полученных данных
       // user_chat::processPacket(m_packetData.data(), m_packetData.size());

        // Чтение следующего заголовка пакета
        readPacketHeader();
    }
    void sendPacket( const uint8_t* message, size_t size )
    {
        boost::asio::async_write(m_socket, boost::asio::buffer(message,size),
                                 [message,self = this->shared_from_this()](const boost::system::error_code& ec, std::size_t length)
                                 {
                                     delete [] message;
                                     if (!ec) {
                                         std::cout << "Sent message: " << length << " bytes\n";
                                     }
                                 });
    }
};

