#include <lobby.h>

bool Lobby::allPlayersReady() const
{
    if (players.empty()) return false;


    for (auto it = players.begin(); it != players.end(); ++it)
    {
        if (!it->second.ready) return false;
    }

    return true;
}

LobbyPlayer& Lobby::getPlayer(uint32_t id) const
{
    return players.at(id);
}

bool Lobby::addPlayer(const LobbyPlayer& lp)
{
    if (players.find(lp.playerID) != players.end())
    {
        return false;
    }
    players[lp.playerID] = lp;
    return true;
}

void Lobby::handleDisconnect(uint32_t playerID)
{
    if (players.find(playerID) == players.end())
    {
        return;
    }

    players.erase(playerID);
}

void Lobby::handleWaitingRoomPacket(const WaitingRoomActionPacket& waitingRoomPacket)
{
    switch (waitingRoomPacket.waitingRoomActionType)
        {
        case WaitingRoomActionType::SetCharacter:
        {
            if (players.find(waitingRoomPacket.playerID) == players.end())
            {
                break;
            }
            players.at(waitingRoomPacket.playerID).charID = waitingRoomPacket.newCharID;
            break;
        }
        case WaitingRoomActionType::SetYear:
        {
            schoolYear = waitingRoomPacket.newYear;
            break;
        }
        case WaitingRoomActionType::ToggleReady:
        {
            if (players.find(waitingRoomPacket.playerID) == players.end())
            {
                break;
            }
            players.at(waitingRoomPacket.playerID).ready = !players.at(waitingRoomPacket.playerID).ready;
            break;

        }
        case WaitingRoomActionType::StartGame:
        {
            createGameStateFromLobby();
            lobbyState = LobbyState::InGame;
            break;
        }
        default:
            break;
    }
}

void Lobby::createGameStateFromLobby()
{
    std::cout << "Game successfully started!\n";
}

bool Lobby::init()
{
    return true;
}
bool Lobby::update()
{
    return true;
}
void Lobby::close()
{
}