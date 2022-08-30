#include <gtest/gtest.h>
#include "CustomServer.cpp"

using namespace std::chrono_literals;

class CustomClient : public last::net::client_interface<CustomMsgTypes>
{
public:
    void PingServer()
    {
        last::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerPing;

        std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

        msg << timeNow;
        Send(msg);
    }

    void MessageAll()
    {
        last::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::MessageAll;
        Send(msg);
    }
};

class ServerTest : public ::testing::Test
{
    protected:
    CustomServer* server;
    std::thread running;
    bool bQuit;

    void SetUp() override
    {
        server = new CustomServer(60000);
        bQuit = false;

        ASSERT_TRUE(server->Start());
        running = std::thread([this](){
            while(bQuit)
                server->Update(-1, true);
        });
        running.detach();
    }
 };

TEST_F(ServerTest, Connection)
{
    CustomClient c1, c2, c3;

    c1.Connect("127.0.0.1", 60000);
    c2.Connect("127.0.0.1", 60000);
    ASSERT_EQ(2, server->GetConnectionsCount()) << "Missing connections!\n";

    c3.Connect("127.0.0.0", 60000);
    ASSERT_EQ(2, server->GetConnectionsCount()) << "Wrong client connected!\n";

    ASSERT_TRUE(c1.IsConnected()) << "Client lost!\n";
    c1.Disconnect();
    ASSERT_FALSE(c1.IsConnected()) << "Client couldn't disconnect!\n";

    delete server;
}

TEST_F(ServerTest, Communication)
{
    CustomClient client;

    ASSERT_TRUE(client.Incoming().empty());

    client.Connect("127.0.0.1", 60000);
    std::this_thread::sleep_for(10ms);

    ASSERT_FALSE(client.Incoming().empty()) << "No message received!\n";
    EXPECT_EQ(1, client.Incoming().count()) << "Missing connection confirmation!\n";

    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerMessage;
    server->MessageAllClients(msg);
    std::this_thread::sleep_for(10ms);

    EXPECT_EQ(2, client.Incoming().count()) << "MessageAllClients Failed!\n";
}