#include <last_net.h>
#include "CustomMessageTypes.h"

class CustomClient : public last::net::client_interface<CustomMsgTypes>
{
public:
    void PingServer();
    void MessageAll();
};
