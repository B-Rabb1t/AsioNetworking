#pragma once
#include <last_net.h>
#include "CustomMessageTypes.h"

class CustomServer : public last::net::server_interface<CustomMsgTypes>
{
protected:
    virtual bool OnClientConnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client) override;
    virtual void OnClientDisconnect(std::shared_ptr<last::net::connection<CustomMsgTypes>> client) override;
    virtual void OnMessage(std::shared_ptr<last::net::connection<CustomMsgTypes>> client, const last::net::message<CustomMsgTypes> &msg) override;

public:
    CustomServer(uint16_t nPort);
    int GetConnectionsCount();
};