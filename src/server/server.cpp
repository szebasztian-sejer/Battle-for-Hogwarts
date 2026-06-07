#include <server.h>
#include <networkPackets.h>
#include <iostream>
#include <cstring>


Server::Server()
{}

Server::~Server()
{}

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

bool Server::addServerPlayer(uint32_t serverPlayerID, ENetPeer* _peer)
{
    if (serverPlayers.find(serverPlayerID) != serverPlayers.end())
    {
        return false;
    }
    ServerPlayer sp;
    sp.peer = _peer;
    sp.playerId = serverPlayerID;
    serverPlayers[serverPlayerID] = sp;
    return true;
}

bool Server::update()
{
    ENetEvent enetEvent = {};
    
    switch (serverState)
    {
        case ServerState::WAITING_FOR_HOST:
        {
            while (enet_host_service(server, &enetEvent, 0) > 0)
            {
                handleWaitingEvent(enetEvent);
            }
            break;
        }
        case ServerState::IN_LOBBY:
        {
            while (enet_host_service(server, &enetEvent, 0) > 0)
            {
                handleLobbyEvent(enetEvent);
                broadcastLobbyState();
            }
            break;
        }
        case ServerState::IN_GAME:
        {
            if (!gameState.update())
            {
                broadcastGameState();
                serverState = ServerState::IN_LOBBY;
                break;
            }
            
            while (enet_host_service(server, &enetEvent, 0) > 0)
            {
                handleGameEvent(enetEvent);
                broadcastGameState();
            }
            break;
        }
        default:
            break;
    }

    return serverState != ServerState::CLOSING;
}

void Server::handleConnect(ENetPeer* peer)
{
    std::cout << "Client connected.\n";
    playerCount++;
}



void Server::handleWaitingEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
    {
        case ENET_EVENT_TYPE_CONNECT:
        {
            if (playerCount < maxPlayerCount)
            {
                handleConnect(enetEvent.peer);
            }
            else
            {
                std::cout << "Server full, disconnecting client.\n";
                enet_peer_disconnect(enetEvent.peer, 0);
            }
            break;
        }

        case ENET_EVENT_TYPE_RECEIVE:
        {
            if (enetEvent.packet->dataLength < sizeof(PacketType))
            {
                std::cout << "Received packet too small\n";
                enet_packet_destroy(enetEvent.packet);
                break;
            }

            PacketType packetType;
            std::memcpy(&packetType, enetEvent.packet->data, sizeof(PacketType));

            if (packetType != PacketType::CreateLobby)
            {
                std::cout << "Received invalid packet\n";
                enet_packet_destroy(enetEvent.packet);
                break;
            }

            enet_packet_destroy(enetEvent.packet);

            std::cout << "Received request to create lobby\n";

            auto host = createLobbyPlayer();
            host.isHost = true;
            assignPlayerID(enetEvent.peer, host.playerId);
            lobbyState.addPlayer(host);
            addServerPlayer(host.playerId, enetEvent.peer);
            serverState = ServerState::IN_LOBBY;
            createLobby();
            broadcastLobbyState();
            break;

        }

        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "Client disconnected\n";
            playerCount--;
            break;
        }

        default:
            break;
    }

}

void Server::createLobby()
{
    CreateLobbyPacket createLobbyPacket;
    ENetPacket* packet = enet_packet_create(
        &createLobbyPacket,
        sizeof(CreateLobbyPacket),
        ENET_PACKET_FLAG_RELIABLE
    );

    std::cout << "Sending back reply to create lobby\n";

    enet_host_broadcast(server, 0, packet);
}

void Server::broadcastGameState()
{
    GameStatePacket gamePacket;
    gamePacket.packetType = PacketType::GameState;
    gamePacket.gameState = gameState;
    ENetPacket* packet = enet_packet_create(
        &gamePacket,
        sizeof(GameStatePacket),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_host_broadcast(server, 0, packet);
}

void Server::broadcastLobbyState()
{
    LobbyStatePacket lobbyPacket;
    lobbyPacket.schoolYear = lobbyState.schoolYear;

    int i = 0;
    for (auto& [id, player] : lobbyState.players)
    {
        if (i >= MAX_PLAYERS) break;

        lobbyPacket.players[i].playerId = player.playerId;
        lobbyPacket.players[i].connected = true;
        lobbyPacket.players[i].ready = player.ready;
        lobbyPacket.players[i].isHost = player.isHost;
        lobbyPacket.players[i].charID = player.charID;

        i++;
    }

    lobbyPacket.playerCount = i;

    ENetPacket* packet = enet_packet_create(
        &lobbyPacket,
        sizeof(LobbyStatePacket),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_host_broadcast(server, 0, packet);
}

void Server::createGameStateFromLobby()
{
    std::cout << "Game successfully started!\n";
}

LobbyPlayer Server::createLobbyPlayer()
{
    LobbyPlayer lobbyPlayer;
    lobbyPlayer.playerId = playerId;
    playerId++;
    return lobbyPlayer;
}

void Server::assignPlayerID(ENetPeer* _peer, uint32_t id)
{
    AssignPlayerIDPacket idPacket;
    idPacket.playerID = id;

    ENetPacket* packet = enet_packet_create(&idPacket, sizeof(AssignPlayerIDPacket), ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(_peer, 0, packet);
}

void Server::handleLobbyEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            if (playerCount < maxPlayerCount)
            {
                handleConnect(enetEvent.peer);
                std::cout << "Received request to join lobby\n";
                auto guest = createLobbyPlayer();
                
                guest.isHost = false;
                assignPlayerID(enetEvent.peer, guest.playerId);
                lobbyState.addPlayer(guest);
                
                addServerPlayer(guest.playerId, enetEvent.peer);
                createLobby();
                broadcastLobbyState();
            }
            else
            {
                std::cout << "Server full, disconnecting client.\n";
                enet_peer_disconnect(enetEvent.peer, 0);
            }
            break;
        }
        case ENET_EVENT_TYPE_RECEIVE:
        {
            handleLobbyPacket(enetEvent.peer, enetEvent.packet);
            enet_packet_destroy(enetEvent.packet);
            broadcastLobbyState();
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            handleLobbyDisconnect(enetEvent.peer);
            break;
        }
        default:
            break;
    }
}

void Server::handleLobbyPacket(ENetPeer* peer, ENetPacket* packet)
{
    if (packet->dataLength < sizeof(LobbyActionPacket))
    {
        std::cout << "Packet too small\n";
        return;
    }

    PacketType packetType;
    std::memcpy(&packetType, packet->data, sizeof(PacketType));

    if (packetType != PacketType::LobbyAction)
    {
        std::cout << "Invalid packet\n";
        return;
    }

    LobbyActionPacket lobbyPacket;
    std::memcpy(&lobbyPacket, packet->data, sizeof(LobbyActionPacket));
    switch (lobbyPacket.actionType)
        {
        case LobbyActionType::PlayerReady:
        {
            auto& player = lobbyState.getPlayer(lobbyPacket.playerId);
            player.ready = true;
            break;
        }
        case LobbyActionType::PlayerUnready:
        {
            auto& player = lobbyState.getPlayer(lobbyPacket.playerId);
            player.ready = false;
            break;
        }
        case LobbyActionType::SetSchoolYear:
        {
            lobbyState.schoolYear = lobbyPacket.schoolYear;
            break;
        }
        case LobbyActionType::SelectCharacter:
        {
            auto& player = lobbyState.getPlayer(lobbyPacket.playerId);
            player.charID = lobbyPacket.charID;
            break;
        }
        case LobbyActionType::StartGame:
        {
            if (lobbyState.allPlayersReady())
            {
                createGameStateFromLobby();
                serverState = ServerState::IN_GAME;
                LobbyActionPacket reply;
                reply.actionType = LobbyActionType::StartGame;

                ENetPacket* replyPacket = enet_packet_create(&reply, sizeof(LobbyActionPacket), ENET_PACKET_FLAG_RELIABLE);
                enet_host_broadcast(server, 0, replyPacket);

                broadcastGameState();
            }
            break;
        }
        default:
            break;       
    }

}

void Server::handleLobbyDisconnect(ENetPeer* peer)
{
    uint32_t key = -1;
    for (auto it = serverPlayers.begin(); it != serverPlayers.end(); ++it)
    {
        if (it->second.peer)
        {
            if (it->second.peer == peer)
            {
                key = it->first;
                break;
            }
        }
    }
    if (key != -1)
    {
        lobbyState.players.erase(key);
        serverPlayers.erase(key);
        playerCount--;
        if (playerCount == 0)
        {
            serverState = ServerState::WAITING_FOR_HOST;
        }
    }

    std::cout << "Client disconnected from lobby\n";
}

void Server::handleGamePacket(ENetPeer* peer, ENetPacket* packet)
{}

void Server::handleGameEvent(ENetEvent& enetEvent)
{}

void Server::handleGameDisconnect(ENetPeer* peer)
{}


void Server::close()
{
    enet_host_destroy(server);
    enet_deinitialize();
}
