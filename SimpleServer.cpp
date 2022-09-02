#include "CustomServer.h"

int main()
{
    CustomServer server(60000);
    server.Start();

    while (1)
    {
        server.Update(-1, true);
    }
}
