#pragma once
#include <enet/enet.h>
#include <cstdint>

struct ServerPlayer
{
    uint32_t playerId = 0;
    ENetPeer* peer = nullptr;
};