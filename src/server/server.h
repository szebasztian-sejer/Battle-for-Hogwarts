#pragma once
#include <serverPlayer.h>
#include <gameState.h>
#include <lobbyState.h>
#include <unordered_map>

class Server
{
public:
    ENetHost* server = nullptr;

    enum class ServerState
    {
        WAITING_FOR_HOST,
        IN_LOBBY,
        IN_GAME,
        CLOSING
    };

    ServerState serverState = ServerState::WAITING_FOR_HOST;

    uint32_t playerId = 0;

    int playerCount = 0;
    const int maxPlayerCount = 4;
    int readyPlayers = 0;

    std::unordered_map<uint32_t, ServerPlayer> serverPlayers;

    GameState gameState;
    LobbyState lobbyState;
    Server();
    bool init();
    bool update();
    void close();

    void handleWaitingEvent(ENetEvent& enetEvent);
    void handleLobbyEvent(ENetEvent& enetEvent);
    void handleGameEvent(ENetEvent& enetEvent);

    void handleConnect(ENetPeer* peer);

    void handleGameDisconnect(ENetPeer* peer);
    void handleLobbyDisconnect(ENetPeer* peer);


    void handleLobbyPacket(ENetPeer* peer, ENetPacket* packet);
    void handleGamePacket(ENetPeer* peer, ENetPacket* packet);

    void createLobby();

    void broadcastGameState();
    void broadcastLobbyState();

    bool addServerPlayer(uint32_t serverPlayerID, ENetPeer* _peer);

    void assignPlayerID(ENetPeer* _peer, uint32_t id);

    LobbyPlayer createLobbyPlayer();

    void createGameStateFromLobby();



    ~Server();
};