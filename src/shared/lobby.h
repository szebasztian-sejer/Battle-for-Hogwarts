#pragma once
#include <lobbyPlayer.h>
#include <unordered_map>
#include <gameState.h>
#include <networkPackets.h>


struct Lobby
{
    uint32_t lobbyId = 0;

    static constexpr int MAX_PLAYERS = 4;
    int schoolYear = 1;

    int playerCount = 0;
    

    enum class LobbyState
    {
        None,
        WaitingRoom,
        InGame,
        Closing,
        LOBBY_STATE_COUNT,
    };
    GameState gameState;
    LobbyState lobbyState = LobbyState::None;

    std::unordered_map<uint32_t, LobbyPlayer> players;

    bool allPlayersReady() const;
    LobbyPlayer& getPlayer(uint32_t id);

    bool addPlayer(const LobbyPlayer& lp);

    void handleWaitingRoomPacket(const WaitingRoomActionPacket& waitingRoomPacket);
    void handleDisconnect(uint32_t playerID);
    void createGameStateFromLobby();

    bool init();
    bool update();
    void close();


};