#include <client.h>
#include <enet/enet.h>
#include <iostream>
#include <cstring>


Client::Client()
{      
}

Client::~Client()
{}

bool Client::init()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet.\n";
        return false;
    }

    client = enet_host_create(
        nullptr,
        1,  // outgoing connections
        2,  // channels
        0,
        0
    );

    if (!client)
    {
        std::cerr << "Failed to create ENet client.\n";
        return false;
    }
   
    return true;
}

bool Client::connect(const char* serverAddress)
{
    ENetAddress address = {};
    enet_address_set_host(&address, serverAddress);
    address.port = 1234;

    peer = enet_host_connect(client, &address, 2, 0);

    if (!peer)
    {
        std::cout << "Failed to start connection.\n";
        return false;
    }

    std::cout << "Connection attempt started.\n";
    return true;
}

bool Client::update()
{
    ENetEvent event = {};

    while (enet_host_service(client, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "Connected to server.\n";
            break;
        }

        case ENET_EVENT_TYPE_RECEIVE:
        {
            std::string msg(
                reinterpret_cast<const char*>(event.packet->data),
                event.packet->dataLength
            );

            std::cout << "Client received: " << msg << "\n";
            enet_packet_destroy(event.packet);
            break;
        }

        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "Disconnected from server.\n";
            peer = nullptr;
            return false;
        }

        default:
            break;
        }
    }

    return true;
}

void Client::close()
{
    enet_host_destroy(client);
    enet_deinitialize();
}

void Client::SendPlayerAction(
    uint32_t playerId,
    ActionType actionType,
    uint32_t cardId,
    uint32_t targetId)
{
    PlayerActionPacket action;

    action.playerId = playerId;
    action.actionType = actionType;
    action.cardInstanceId = cardId;
    action.targetId = targetId;

    ENetPacket* packet = enet_packet_create(
        &action,
        sizeof(PlayerActionPacket),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(peer, 0, packet);
}
