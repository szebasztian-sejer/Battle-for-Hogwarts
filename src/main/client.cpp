#include <iostream>
#include <cstring>
#include <enet/enet.h>

int main()
{
    if (enet_initialize() != 0)
    {
        std::cerr << "Failed to initialize ENet.\n";
        return 1;
    }

    ENetHost* client = enet_host_create(
        nullptr,
        1,  // outgoing connections
        2,  // channels
        0,
        0
    );

    if (!client)
    {
        std::cerr << "Failed to create ENet client.\n";
        return 1;
    }

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);

    if (!peer)
    {
        std::cerr << "Failed to start connection.\n";
        return 1;
    }

    ENetEvent event;

    if (enet_host_service(client, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        std::cout << "Connected to server.\n";

        const char* msg = "Hello from client!";
        ENetPacket* packet = enet_packet_create(
            msg,
            strlen(msg) + 1,
            ENET_PACKET_FLAG_RELIABLE
        );

        enet_peer_send(peer, 0, packet);
    }
    else
    {
        std::cout << "Connection failed.\n";
        enet_peer_reset(peer);
        return 1;
    }

    while (true)
    {
        while (enet_host_service(client, &event, 1000) > 0)
        {
            switch (event.type)
            {
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
                return 0;
            }
            default:
                break;
            }
        }
    }

    enet_host_destroy(client);
    enet_deinitialize();
}