#pragma once
#include <serverPlayer.h>
#include <lobby.h>
#include <unordered_map>

class Server
{
public:
    ENetHost* server = nullptr;

    uint32_t playerID = 0;

    int playerCount = 0;
    const int maxPlayerCount = 4;
   
    std::unordered_map<uint32_t, Lobby> lobbies;
    std::unordered_map<uint32_t, ServerPlayer> serverPlayers;

    bool init();
    bool update();
    void close();

    void handleConnect(ENetPeer* peer);
    void handleDisconnect(ENetPeer* peer);

    void handleEvent(ENetEvent& enetEvent);
    uint32_t Server::getPlayerID(ENetPeer* peer) const;

    uint32_t createLobby();
    void inviteToLobby(ENetPeer* peer, uint32_t lobbyId);
    void broadcastLobbyState(uint32_t lobbyID);

    void handlePacket(ENetPeer* peer, ENetPacket* packet);
    bool addServerPlayer(uint32_t serverPlayerID, ENetPeer* _peer);
    void assignPlayerID(ENetPeer* peer, uint32_t id);

    LobbyPlayer createLobbyPlayer();

};