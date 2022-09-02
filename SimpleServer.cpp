#include "CustomServer.h"

CustomServer* CustomServer::s_instance = nullptr;

int main()
{
    CustomServer::instance(60000)->Start();

    while (1)
    {
        CustomServer::instance()->Update(-1, true);
    }
}
