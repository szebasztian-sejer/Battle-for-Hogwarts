#include <lobbyState.h>

bool LobbyState::allPlayersReady() const
{
    if (players.empty()) return false;


    for (auto it = players.begin(); it != players.end(); ++it)
    {
        if (!it->second.ready) return false;
    }

    return true;
}

LobbyPlayer& LobbyState::getPlayer(uint32_t id)
{
    return players.at(id);
}

bool LobbyState::addPlayer(const LobbyPlayer& lp)
{
    if (players.find(lp.playerId) != players.end())
    {
        return false;
    }
    players[lp.playerId] = lp;
    return true;
}