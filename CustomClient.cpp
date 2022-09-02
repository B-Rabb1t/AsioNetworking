#include <last_net.h>
#include "CustomClient.h"

void CustomClient::PingServer()
{
    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::ServerPing;

    std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();

    msg << timeNow;
    Send(msg);
}

void CustomClient::MessageAll()
{
    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::MessageAll;
    Send(msg);
}
