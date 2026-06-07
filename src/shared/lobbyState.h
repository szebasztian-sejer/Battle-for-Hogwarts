#pragma once
#include <lobbyPlayer.h>
#include <unordered_map>
#include <iterator>
#include <algorithm>

struct LobbyState
{
    uint32_t lobbyId = 0;

    int maxPlayers = 4;
    int schoolYear = 1;

    std::unordered_map<uint32_t,LobbyPlayer> players;

    bool allPlayersReady() const;
    LobbyPlayer& getPlayer(uint32_t id);

    bool addPlayer(const LobbyPlayer& lp);
};