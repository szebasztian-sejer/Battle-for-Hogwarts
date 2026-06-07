#pragma once
#include <string>
#include <characters.h>

struct LobbyPlayer
{
    uint32_t playerId = 0;
    bool ready = false;
    bool connected = false;
    bool isHost = false;
    CharacterID charID = CharacterID::None;
};