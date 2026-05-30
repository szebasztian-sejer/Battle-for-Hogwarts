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

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;

    ENetHost* server = enet_host_create(
        &address,
        4,      // max clients
        2,      // channels
        0,      // incoming bandwidth limit
        0       // outgoing bandwidth limit
    );

    if (!server)
    {
        std::cerr << "Failed to create ENet server.\n";
        return 1;
    }

    std::cout << "Server started on port 1234.\n";

    ENetEvent event;

    while (true)
    {
        while (enet_host_service(server, &event, 1000) > 0)
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
                std::string msg(
                    reinterpret_cast<const char*>(event.packet->data),
                    event.packet->dataLength
                );
                std::cout << "Server received: " << msg << "\n";
                enet_packet_destroy(event.packet);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
            {
                std::cout << "Client disconnected.\n";
                break;
            }

            default:
                break;
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();
}