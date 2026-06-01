#pragma once
#include <cstdint>

enum class PacketType : uint8_t
{
    None = 0,
    PlayerAction = 1,
    GameState = 2,
    ChatMessage = 3
};

enum class ActionType : uint8_t
{
    None = 0,
    PlayCard = 1,
    EndTurn = 2
};

struct PlayerActionPacket
{
    PacketType packetType = PacketType::PlayerAction;

    uint32_t playerId = 0;
    ActionType actionType = ActionType::None;

    uint32_t cardInstanceId = 0;
    uint32_t targetId = 0;
};