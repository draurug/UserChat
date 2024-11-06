#pragma once

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "Logs.h"

class IAppliedTcpSession {
public:
    virtual void onPacketReceived(const std::vector<uint8_t>& packetData) = 0;
    virtual ~IAppliedTcpSession() = default; // Добавляем виртуальный деструктор
};

class TcpClientSession : public std::enable_shared_from_this<TcpClientSession>, public IAppliedTcpSession {
protected:
    boost::asio::ip::tcp::socket m_socket;

    uint16_t m_dataLength;
    std::vector<uint8_t> m_packetData;

public:
    TcpClientSession(boost::asio::ip::tcp::socket&& socket)
        : m_socket(std::move(socket)) {}

    void write(const uint8_t* response, size_t dataSize) {
        auto self = shared_from_this(); // Сохраняем shared_ptr
        boost::asio::async_write(m_socket, boost::asio::buffer(response, dataSize),
                                [self, response](const boost::system::error_code& error, std::size_t sentSize) {
                                    LOG_ERR("TcpClientSession sentSize: " << sentSize);
                                    delete[] response; // Освобождаем память, если выделено динамически
                                    if (error) {
                                        LOG_ERR("TcpClientSession async_send error: " << error.message());
                                    }
                                });
    }

    void readPacketHeader() {
        auto self = shared_from_this(); // Сохраняем shared_ptr

        boost::asio::async_read(m_socket, boost::asio::buffer(&m_dataLength, sizeof(m_dataLength)),
                                [self](const boost::system::error_code& error, std::size_t bytesTransferred) {
                                    self->doReadPacketHeader(error, bytesTransferred);
                                });
    }

    void doReadPacketHeader(const boost::system::error_code& error, std::size_t bytesTransferred) {
        if (error) {
            LOG_ERR("TcpClientSession read error: " << error.message());
            return;
        }
        if (bytesTransferred != sizeof(m_dataLength)) {
            LOG_ERR("TcpClientSession read error (m_dataLength): " << bytesTransferred << " vs " << sizeof(m_dataLength));
            return;
        }

        LOG("Received packet length: " << m_dataLength);
        m_packetData.resize(m_dataLength);

        auto self = shared_from_this();
        boost::asio::async_read(m_socket, boost::asio::buffer(m_packetData.data(), m_dataLength),
                                [self](const boost::system::error_code& error, std::size_t bytesTransferred) {
                                    self->readPacketData(error, bytesTransferred);
                                });
    }

    void readPacketData(const boost::system::error_code& error, std::size_t bytesTransferred) {
        if (error) {
            LOG_ERR("TcpClientSession read error: " << error.message());
            return; // Обрабатываем ошибку, но не останавливаем сервер
        }
        if (bytesTransferred != m_dataLength) {
            LOG_ERR("TcpClientSession read error (bytes_transferred): " << bytesTransferred << " vs " << m_dataLength);
            return; // Обрабатываем ошибку, но не останавливаем сервер
        }

        onPacketReceived(m_packetData); // Вызываем обработчик пакета

        readPacketHeader(); // Читаем следующий заголовок пакета
    }

    // Реализация метода IAppliedTcpSession
    void onPacketReceived(const std::vector<uint8_t>& packetData) override {
        // Обработка полученного пакета
        LOG("Packet received: " << std::string(packetData.begin(), packetData.end()));
    }
};

class TcpServer {
    boost::asio::io_context m_context;
    boost::asio::ip::tcp::endpoint m_endpoint;
    std::optional<boost::asio::ip::tcp::acceptor> m_acceptor;

public:
    TcpServer(const std::string& addr, const std::string& port)
        : m_context(),
        m_acceptor(boost::asio::ip::tcp::acceptor(m_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(addr), std::stoi(port)))) {
        LOG("TcpServer initialized on " << addr << ":" << port);
    }

    void run() {
        asyncAccept();
        m_context.run();
    }

    void shutdown() {
        m_context.stop();
        LOG("TcpServer shutdown");
    }

private:
    void asyncAccept() {
        m_acceptor->async_accept([this](boost::system::error_code errorCode, boost::asio::ip::tcp::socket socket) {
            if (errorCode) {
                LOG_ERR("async_accept error: " << errorCode.message());
            } else {
                LOG("New connection accepted");
                auto session = std::make_shared<TcpClientSession>(std::move(socket));
                session->readPacketHeader(); // Начинаем чтение заголовка пакета
            }
            asyncAccept(); // Продолжаем принимать новые подключения
        });
    }
};

