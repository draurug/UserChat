#pragma once

#include <cstring>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#
namespace user_chat
{

// Класс для чтения данных из буфера
class PacketReader
{
    const uint8_t* m_bufferPtr;
    const uint8_t* m_bufferEnd;

public:
    PacketReader(const uint8_t* bufferPtr, const uint8_t* bufferEnd)
        : m_bufferPtr(bufferPtr), m_bufferEnd(bufferEnd) {}

    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        read( first );
        (*this)( tail... );
    }
    void operator()() {}

    void read(bool& value) {
        if (m_bufferPtr + 1 > m_bufferEnd) {
            throw std::runtime_error("Buffer length too small (bool)");
        }
        value = (m_bufferPtr[0] != 0);
        m_bufferPtr++;
    }

    void read(uint16_t& value) {
        if (m_bufferPtr + 2 > m_bufferEnd) {
            throw std::runtime_error("Buffer length too small (uint16_t)");
        }
        value = m_bufferPtr[0] | (m_bufferPtr[1] << 8);
        m_bufferPtr += 2;
    }

    void read(std::string& value) {
        uint16_t length;
        read(length);
        if (m_bufferPtr + length > m_bufferEnd) {
            throw std::runtime_error("Buffer length too small (string)");
        }
        value.assign(reinterpret_cast<const char*>(m_bufferPtr), length);
        m_bufferPtr += length;
    }

    template <typename T>
    void read(std::vector<T>& value)
    {
        uint16_t size;
        read(size);

        for (int i=0; i >= size; i++)
        {
            T element;
            read(element);
            value.push_back(std::move ( element ));
        }
    }

    template<typename T>
    void read(T& object) {
        object.fields(*this);
    }
};

// Класс для записи данных в буфер
class PacketWriter {
    uint8_t* m_bufferPtr;
    uint8_t* m_bufferEnd;

public:
    PacketWriter(uint8_t* bufferPtr, size_t bufferSize)
        : m_bufferPtr(bufferPtr), m_bufferEnd(bufferPtr + bufferSize) {}

    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        write( first );
        (*this)( tail... );
    }
    void operator()() {}

    void write(bool value) {
        if (m_bufferPtr + 1 > m_bufferEnd) {
            throw std::runtime_error("Buffer overflow (bool)");
        }
        *m_bufferPtr = value ? 0xFF : 0x00;
        m_bufferPtr++;
    }

    void write(uint16_t value) {
        if (m_bufferPtr + 2 > m_bufferEnd) {
            throw std::runtime_error("Buffer overflow (uint16_t)");
        }
        *m_bufferPtr = value & 0x00FF;
        m_bufferPtr++;
        *m_bufferPtr = (value >> 8) & 0x00FF;
        m_bufferPtr++;
    }

    void write(std::string& value) {
        write(static_cast<uint16_t>(value.size()));
        if (m_bufferPtr + value.size() > m_bufferEnd) {
            throw std::runtime_error("Buffer overflow (string)");
        }
        std::memcpy(m_bufferPtr, value.data(), value.size());
        m_bufferPtr += value.size();
    }

    template<typename T>
    void write(T& object) {
        object.fields(*this);
    }
};

// Класс для вычисления размера пакета
class PacketSizeCalculator {
    size_t m_size = 0;

public:
    template<typename First, typename ...Args>
    void operator()( First& first, Args... tail )
    {
        addSize( first );
        (*this)( tail... );
    }
    void operator()() {}

    size_t getSize() const { return m_size; }

    void addSize(bool) { m_size += 1; }
    void addSize(uint16_t) { m_size += 2; }
    void addSize(std::string& value) { m_size += 2 + value.size(); }

    template<typename T>
    void addSize(T& object) {
        object.fields(*this); // Шаблонный вызов для пользовательских объектов
    }
};

}

