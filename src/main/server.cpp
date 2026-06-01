#include <server.h>
#include <enet/enet.h>
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

    server = enet_host_create(&address, 4, 2, 0, 0);

    if (!server)
    {
        std::cerr << "Failed to create ENet server.\n";
        return false;
    }

    std::cout << "Server started on port 1234.\n";
    return true;
}

bool Server::update()
{
    ENetEvent event = {};

    while (enet_host_service(server, &event, 0) > 0)
    {
        switch (event.type)
        {
        case ENET_EVENT_TYPE_CONNECT:
        {
            std::cout << "Client connected.\n";

            {
                const char* msg = "Hello from server!";
                ENetPacket* packet = enet_packet_create(
                    msg,
                    strlen(msg) + 1,
                    ENET_PACKET_FLAG_RELIABLE
                );

                enet_peer_send(event.peer, 0, packet);
            }
            break;
        }


        case ENET_EVENT_TYPE_RECEIVE:
        {
            if (event.packet->dataLength < sizeof(PacketType))
            {
                std::cout << "Received packet too small.\n";
                enet_packet_destroy(event.packet);
                break;
            }

            PacketType packetType;
            std::memcpy(&packetType, event.packet->data, sizeof(PacketType));

            switch (packetType)
            {
            case PacketType::PlayerAction:
            {
                if (event.packet->dataLength != sizeof(PlayerActionPacket))
                {
                    std::cout << "Invalid PlayerActionPacket size. Got "
                        << event.packet->dataLength
                        << ", expected "
                        << sizeof(PlayerActionPacket)
                        << "\n";
                    break;
                }

                PlayerActionPacket action;
                std::memcpy(&action, event.packet->data, sizeof(PlayerActionPacket));

                std::cout << "Received PlayerActionPacket:\n";
                std::cout << "  playerId: " << action.playerId << "\n";
                std::cout << "  actionType: " << static_cast<int>(action.actionType) << "\n";
                std::cout << "  cardInstanceId: " << action.cardInstanceId << "\n";
                std::cout << "  targetId: " << action.targetId << "\n";

                // Later:
                // if (IsActionLegal(gameState, action))
                // {
                //     ApplyAction(gameState, action);
                //     BroadcastGameState(server, gameState);
                // }

                break;
            }

            default:
                std::cout << "Unknown packet type: "
                    << static_cast<int>(packetType)
                    << "\n";
                break;
            }

            enet_packet_destroy(event.packet);
            break;
        }
        break;

        case ENET_EVENT_TYPE_DISCONNECT:
        {
            std::cout << "Client disconnected.\n";
            return false;
        }

        default:
            break;
        }
    }
    return true;
}

void Server::close()
{
    enet_host_destroy(server);
    enet_deinitialize();
}