#include "TcpServer.h"
#include "TcpClient.h"

// lvalue = rvalue (movable)
// rvalue = std::move(lvalue)

class ChatServer{};
class ChatSession
{
public:
    void onPacketReceived( const std::vector<uint8_t>& packetData){}
};
class ChatClientBase
{
public:
    void initClient(){}
protected:
    virtual void doWrite( const uint8_t* message, size_t size ) = 0;

};
int main()
{
    std::thread( []
                {
        TcpServer<ChatServer, ChatSession> server("0.0.0.0", "15001" );
                    server.run();
                }).detach();


    auto client = std::make_shared<TcpClient<ChatClientBase>>();

    std::thread clientThread( [&client]
                             {
                                 client->run( "localhost", "15001" );
                             });

    sleep(1);

    char message[] = "001234567890";
    message[0] = 10;
    message[1] = 0;
    client->write( (uint8_t *)message, sizeof(message) );

    clientThread.join();

    return 0;
}
