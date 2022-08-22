#include <iostream>
#include <last_net.h>

enum class CustomMsgTypes : uint32_t
{
    ServerAccept,
    ServerDeny,
    ServerPing,
    MessageAll,
    ServerMessage,
};

class CustomServer : public last::net::server_interface<CustomMsgTypes>
{
protected:
    virtual bool OnClientConnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client) override
    {
        last::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerAccept;
        client->Send(msg);
        return true;
    }

    virtual void OnClientDisconnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client) override
    {
        std::cout << "Removing client [" << client->GetID() << "]\n";
    }

    virtual void OnMessage(std::shared_ptr<last::net::connection<CustomMsgTypes>> client, const last::net::message<CustomMsgTypes> &msg) override
    {
        switch (msg.header.id)
        {
        case CustomMsgTypes::ServerPing:
        {
            std::cout << "[" << client->GetID() << "]: Server Ping\n";
            client->Send(msg);
        }
        break;

        case CustomMsgTypes::MessageAll:
        {
            std::cout << "[" << client->GetID() << "]: Message All\n";
            last::net::message<CustomMsgTypes> msg;
            msg.header.id = CustomMsgTypes::ServerMessage;
            msg << client->GetID();
            MessageAllClients(msg, client);
        }
        break;
        }
    }

public:
    CustomServer(uint16_t nPort) : last::net::server_interface<CustomMsgTypes>(nPort) {}
};

int main()
{
    CustomServer server(60000);
    server.Start();

    while (1)
    {
        server.Update(-1, true);
    }
}
