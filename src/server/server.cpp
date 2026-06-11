#include <server.h>
#include <networkPackets.h>
#include <iostream>
#include <cstring>
#include <helpers.h>
#include <thread>
#include <chrono>

bool Server::init()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet.\n";
        return false;
    }

    ENetAddress address = {};
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    server = enet_host_create(&address, maxPlayerCount, 2, 0, 0);

    if (!server)
    {
        std::cerr << "Failed to create ENet server.\n";
        return false;
    }

    std::cout << "Server started on port 1234.\n";
    return true;
}

bool Server::addServerPlayer(uint32_t serverPlayerID, ENetPeer* peer)
{
    ServerPlayer sp;
    sp.peer = peer;
    sp.playerID = serverPlayerID;
    serverPlayers[serverPlayerID] = sp;
    return true;
}

uint32_t Server::getPlayerID(ENetPeer* peer) const
{
    for (auto it = serverPlayers.begin(); it != serverPlayers.end(); ++it)
    {
        if (it->second.peer == peer) { return it->first; }
    }
}



void Server::handleConnect(ENetPeer* peer)
{
    std::cout << "Client connected\n";
}

void Server::handleDisconnect(ENetPeer* peer)
{
    uint32_t id = getPlayerID(peer);
    for (auto it = lobbies.begin(); it != lobbies.end(); ++it)
    {
        it->second.handleDisconnect(id);
    }
    serverPlayers.erase(id);
}

void Server::handleEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            handleConnect(enetEvent.peer);
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            handlePacket(enetEvent.peer, enetEvent.packet);
            enet_packet_destroy(enetEvent.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            handleDisconnect(enetEvent.peer);
        }
    }
}

LobbyPlayer Server::createLobbyPlayer()
{
    LobbyPlayer lobbyPlayer;
    lobbyPlayer.playerID = playerID;
    playerID++;
    return lobbyPlayer;
}

void Server::assignPlayerID(ENetPeer* peer, uint32_t id)
{
    AssignPlayerIDPacket idPacket;
    idPacket.playerID = id;

    ENetPacket* packet = enet_packet_create(&idPacket, sizeof(AssignPlayerIDPacket), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);
}

void Server::handlePacket(ENetPeer* peer, ENetPacket* packet)
{
    PacketType packetType;

    std::memcpy(&packetType, packet->data, sizeof(PacketType));
    switch (packetType)
    {
        case PacketType::HostLobby:
        {
            auto host = createLobbyPlayer();
            host.isHost = true;
            assignPlayerID(peer, host.playerID);
            uint32_t id = createLobby();
            addServerPlayer(host.playerID, peer);
            lobbies.at(id).addPlayer(host);
 
            broadcastLobbyState(id);
            break;
        }
        case PacketType::JoinLobby:
        {
            auto player = createLobbyPlayer();
            assignPlayerID(peer, player.playerID);
            JoinLobbyPacket joinLobbyPacket;
            std::memcpy(&joinLobbyPacket, packet->data, sizeof(JoinLobbyPacket));
            lobbies.at(joinLobbyPacket.lobbyID).addPlayer(player);

            broadcastLobbyState(joinLobbyPacket.lobbyID);
            break;
        }

        case PacketType::WaitingRoomAction:
        {
            WaitingRoomActionPacket actionPacket;
            std::memcpy(&actionPacket, packet->data, sizeof(WaitingRoomActionPacket));
            uint32_t lobbyID = actionPacket.lobbyID;
            if (lobbies.find(lobbyID) == lobbies.end())
            {
                std::cout << "Lobby ID not found!\n";
                return;
            }
            lobbies.at(lobbyID).handleWaitingRoomPacket(actionPacket);
            break;
        }
    }

}

void Server::broadcastLobbyState(uint32_t lobbyID)
{
    if (lobbies.find(lobbyID) == lobbies.end()) { return; }

    Lobby lobby = lobbies.at(lobbyID);
    ENetPacket* packet = nullptr;

    if (lobby.lobbyState == Lobby::LobbyState::WaitingRoom)
    {
        WaitingRoomStatePacket waitingRoomPacket;
        waitingRoomPacket.schoolYear = lobby.schoolYear;

        int i = 0;
        for (auto& [id, player] : lobby.players)
        {
            if (i >= MAX_PLAYERS) break;

            waitingRoomPacket.players[i].playerID = player.playerID;
            waitingRoomPacket.players[i].connected = true;
            waitingRoomPacket.players[i].ready = player.ready;
            waitingRoomPacket.players[i].isHost = player.isHost;
            waitingRoomPacket.players[i].charID = player.charID;

            i++;
        }

        waitingRoomPacket.playerCount = i;

        packet = enet_packet_create(
            &waitingRoomPacket,
            sizeof(WaitingRoomStatePacket),
            ENET_PACKET_FLAG_RELIABLE
        );

    }
    if (!packet) { return; }
    for (auto& [id, player] : serverPlayers)
    {
        if (lobbies.at(lobbyID).players.find(id) != lobbies.at(lobbyID).players.end())
        {
            enet_peer_send(player.peer, 0, packet);
        }
    }
}

bool Server::update()
{
    for (auto it = lobbies.begin(); it != lobbies.end(); it++)
    {
        it->second.update();
    }

    ENetEvent enetEvent = {};

    while (enet_host_service(server, &enetEvent, 0) > 0)
    {
        handleEvent(enetEvent);
        for (auto it = lobbies.begin(); it != lobbies.end(); it++)
        {
            broadcastLobbyState(it->first);
        }
    }
    
    
    return true;
}



void Server::close()
{
    enet_host_destroy(server);
    enet_deinitialize();
}