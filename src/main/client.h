#pragma once
#include <networkPackets.h>

struct _ENetHost;
struct _ENetPeer;

using ENetHost = _ENetHost;
using ENetPeer = _ENetPeer;

class Client
{
public:
    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;

    Client();
    bool init();
    bool connect(const char* serverAddress);
    bool update();
    void close();

    void SendPlayerAction(
        uint32_t playerId,
        ActionType actionType,
        uint32_t cardId,
        uint32_t targetId);

    ~Client();
};