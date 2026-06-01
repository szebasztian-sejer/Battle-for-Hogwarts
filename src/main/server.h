#pragma once

struct _ENetHost;
using ENetHost = _ENetHost;

class Server
{
public:
    ENetHost* server = nullptr;

    Server();
    bool init();
    bool update();
    void close();

    ~Server();
};