#include "CustomServer.h"

CustomServer::CustomServer(uint16_t nPort) : last::net::server_interface<CustomMsgTypes>(nPort) {}

int CustomServer::GetConnectionsCount()
{
    return m_deqConnections.size();
}

bool CustomServer::OnClientConnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client)
{
    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerAccept;
    client->Send(msg);
    return true;
}

void CustomServer::OnClientDisconnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client)
{
    std::cout << "Removing client [" << client->GetID() << "]\n";
}

void CustomServer::OnMessage(std::shared_ptr<last::net::connection<CustomMsgTypes>> client, const last::net::message<CustomMsgTypes> &msg)
{
    switch (msg.header.id)
    {
    case CustomMsgTypes::ServerPing:
    {
        std::cout << "[" << client->GetID() << "]: Server Ping\n";
        client->Send(msg);
        break;
    }

    case CustomMsgTypes::MessageAll:
    {
        std::cout << "[" << client->GetID() << "]: Message All\n";
        last::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::ServerMessage;
        msg << client->GetID();
        MessageAllClients(msg, client);
        break;
    }
    }
}