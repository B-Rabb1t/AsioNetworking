#include <gtest/gtest.h>
#include "CustomServer.cpp"

using namespace std::chrono_literals;

class CustomClient : public last::net::client_interface<CustomMsgTypes>
{
public:
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
            while(!bQuit)
                server->Update();
        });
    }

    void TearDown() override
    {
        bQuit = true;
        running.join();
        delete server;
    }
 };


/**
 * @brief Verify the connection is succesful only for the right clients
 * The 2 valid clients should connect succesfully
 * The other one shouldn't be able to connect
 * The first client should disconnect successfully
 */
TEST_F(ServerTest, Connection)
{
    CustomClient c1, c2, c3;

    c1.Connect("127.0.0.1", 60000);
    c2.Connect("127.0.0.1", 60000);
    std::this_thread::sleep_for(10ms);
    ASSERT_EQ(2, server->GetConnectionsCount()) << "Missing connections!\n";

    c3.Connect("127.0.0.0", 60000);
    std::this_thread::sleep_for(10ms);
    ASSERT_EQ(2, server->GetConnectionsCount()) << "Wrong client connected!\n";

    ASSERT_TRUE(c1.IsConnected()) << "Client lost!\n";
    c1.Disconnect();
    ASSERT_FALSE(c1.IsConnected()) << "Client couldn't disconnect!\n";
}

/**
 * @brief Verify that every message is received
 * Before connecting the client shouldn't have any messages
 * After connecting the client should receive a confirmation message
 * The client will receive one more message after the sever sends one 
 * to every client connected
 */
TEST_F(ServerTest, Communication)
{
    CustomClient client;

    ASSERT_TRUE(client.Incoming().empty());

    client.Connect("127.0.0.1", 60000);
    std::this_thread::sleep_for(10ms);

    ASSERT_FALSE(client.Incoming().empty()) << "No message received!\n";
    ASSERT_EQ(1, client.Incoming().count()) << "Missing connection confirmation!\n";

    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerMessage;
    server->MessageAllClients(msg);
    std::this_thread::sleep_for(10ms);

    ASSERT_EQ(2, client.Incoming().count()) << "MessageAllClients Failed!\n";
}