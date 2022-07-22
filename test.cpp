#include <iostream>
#include <last_net.h>

enum class CustomMsgTypes : uint32_t
{
    FireBullet,
    MovePlayer
};

int main()
{
    last::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::FireBullet;

    int a = 3;
    bool b = true;
    float c = 3.1415f;

    struct
    {
        float x{};
        float y{};
    }d[5];

    msg << a << b << c << d;

    a = 0;
    b = false;
    c = -1.0f;

    msg >> d >> c >> b >> a;

    std::cout << "Ok\n";
    
    return 0;
}