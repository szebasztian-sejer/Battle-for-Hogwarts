#include <client.h>
#include <characters.h>
#include <iostream>
#include <cstring>
#include <enet/enet.h>

Client::Client()
{      
}

Client::~Client()
{}

bool Client::init()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1024, 860, "Battle for Hogwarts");

    InitAudioDevice();
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);

    mainMenu.drawRec = { 0.0f,0.0f,(float)GetScreenWidth(),(float)GetScreenHeight() };
    lobbyUI.drawRec = { 0.0f,0.0f,(float)GetScreenWidth(),(float)GetScreenHeight() };

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

    enet_peer_timeout(peer, 0, 10000, 30000);

    std::cout << "Connection attempt started.\n";
    return true;
}

bool Client::updateNetwork()
{
    ENetEvent enetEvent = {};

    while (enet_host_service(client, &enetEvent, 0) > 0)
    {
        switch (clientState)
        {
        case ClientState::IN_MENU:
        {
            handleMenuEvent(enetEvent);
            break;
        }

        case ClientState::IN_LOBBY:
        {
            handleLobbyEvent(enetEvent);
            auto it = lobbyState.players.find(playerID);
            if (it != lobbyState.players.end())
            {
                isHost = it->second.isHost;
            }
            break;
        }

        case ClientState::IN_GAME:
        {
            handleGameEvent(enetEvent);
            break;
        }

        default:
            break;
        }
    }

    return clientState != ClientState::CLOSING;
}

void Client::updateUI()
{
    displayAndInteract();
}

#if 0
bool Client::update()
{
    
    ENetEvent enetEvent = {};
    while (enet_host_service(client, &enetEvent, 0) > 0)
    {
        switch (clientState)
        {
        case ClientState::IN_MENU:
        {
            handleMenuEvent(enetEvent);
            break;
        }
        case ClientState::IN_LOBBY:
        {
            handleLobbyEvent(enetEvent);
            auto it = lobbyState.players.find(playerID);
            if (it != lobbyState.players.end())
            {
                isHost = it->second.isHost;
            }
            break;
        }
        case ClientState::IN_GAME:
        {
            handleGameEvent(enetEvent);
            break;
        }
        default:
            break;
        }

    }
    displayAndInteract();

    return true;
}
#endif

void Client::handleMenuEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
    {
    case ENET_EVENT_TYPE_CONNECT:
    {
        std::cout << "Connected to server.\n";
        connectedToServer = true;

        if (wantsToCreateLobby)
        {
            std::cout << "Sending lobby packet\n";

            CreateLobbyPacket createLobbyPacket;
            ENetPacket* packet = enet_packet_create(
                &createLobbyPacket,
                sizeof(CreateLobbyPacket),
                ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(peer, 0, packet);
            enet_host_flush(client);
        }

        break;
    }

    case ENET_EVENT_TYPE_RECEIVE:
    {
        handleMenuPacket(enetEvent.peer, enetEvent.packet);
        enet_packet_destroy(enetEvent.packet);
        break;
    }

    default:
        break;
    }
}

void Client::handleMenuPacket(ENetPeer* peer, ENetPacket* packet)
{
    PacketType packetType;

    std::memcpy(&packetType, packet->data, sizeof(PacketType));

    switch (packetType)
    {
    case PacketType::CreateLobby:
    {
        std::cout << "Changing into lobby\n";
        clientState = ClientState::IN_LOBBY;
        return;
    }
    case PacketType::AssignPlayerID:
    {
        AssignPlayerIDPacket idpacket;
        std::memcpy(&idpacket, packet->data, sizeof(AssignPlayerIDPacket));
        playerID = idpacket.playerID;

        std::cout << "Received player ID: "<<playerID<<"\n";
        break;
    }
    default:
        std::cout << "Received invalid packet\n";
        break;
    }

   
    
}

void Client::handleLobbyEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
        {
            handleLobbyPacket(enetEvent.peer, enetEvent.packet);
            enet_packet_destroy(enetEvent.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
            clientState = ClientState::IN_MENU;
            connectionStarted = false;
            connectedToServer = false;
            wantsToCreateLobby = false;
            break;
        }
        default:
            break;
    }
}

void Client::handleLobbyPacket(ENetPeer* peer, ENetPacket* packet)
{
    PacketType packetType;
    std::memcpy(&packetType, packet->data, sizeof(PacketType));

    switch (packetType)
    {
    case PacketType::LobbyAction:
    {
        LobbyActionPacket action;
        std::memcpy(&action, packet->data, sizeof(LobbyActionPacket));
        if (action.actionType == LobbyActionType::StartGame)
        {
            clientState = ClientState::IN_GAME;
            
        }
        break;
    }
    case PacketType::LobbyState:
    {
        LobbyStatePacket statePacket;
        std::memcpy(&statePacket, packet->data, sizeof(LobbyStatePacket));

        lobbyState.schoolYear = statePacket.schoolYear;
        lobbyState.players.clear();

        for (int i = 0; i < statePacket.playerCount; ++i)
        {
            LobbyPlayer lp;
            lp.playerId = statePacket.players[i].playerId;
            lp.ready = statePacket.players[i].ready;
            lp.isHost = statePacket.players[i].isHost;
            lp.charID = statePacket.players[i].charID;

            lobbyState.players[lp.playerId] = lp;
        }

        break;
    }
    default:
        break;
    }

}

void Client::sendLobbyAction(
    uint32_t playerId, 
    LobbyActionType lobbyAction, 
    int schoolYear, 
    CharacterID charID)
{
    LobbyActionPacket lobbyActionPacket;
    lobbyActionPacket.actionType = lobbyAction;
    lobbyActionPacket.playerId = playerId;
    lobbyActionPacket.schoolYear = schoolYear;
    lobbyActionPacket.charID = charID;

    ENetPacket* packet = enet_packet_create(
        &lobbyActionPacket,
        sizeof(LobbyActionPacket),
        ENET_PACKET_FLAG_RELIABLE
    );

    enet_peer_send(peer, 0, packet);

}


void Client::handleGameEvent(ENetEvent& enetEvent)
{}

void Client::handleGamePacket(ENetPeer* peer, ENetPacket* packet)
{}

void Client::close()
{
    enet_host_destroy(client);
    enet_deinitialize();
}

void Client::displayAndInteract()
{
    switch (clientState)
    {
    case ClientState::IN_MENU:
    {
        mainMenu.widgets.clear();

        if (mainMenu.addButton("Host game"))
        {
            wantsToCreateLobby = true;

            if (!connectionStarted)
            {
                connectionStarted = connect(ipAddress.c_str());
            }
        }

        mainMenu.addTextBox(ipAddress);

        if (mainMenu.addButton("Connect"))
        {
            wantsToCreateLobby = false;

            if (!connectionStarted)
            {
                connectionStarted = connect(ipAddress.c_str());
            }
        }

        mainMenu.updateAndRenderWidgets();

        if (mainMenu.widgets.size() > 1 &&
            mainMenu.widgets[1].type == WidgetTypes::TextBox)
        {
            ipAddress = mainMenu.widgets[1].text;
        }

        mainMenu.lastFrameWidgets = mainMenu.widgets;

        break;
    }
    case ClientState::IN_LOBBY:
    {
        lobbyUI.widgets.clear();

        if (isHost)
        {
            if (lobbyUI.addButton("Year 1"))
            {
                changeSchoolYear(1);
            }
            if (lobbyUI.addButton("Year 2"))
            {
                changeSchoolYear(2);
            }
            if (lobbyUI.addButton("Year 3"))
            {
                changeSchoolYear(3);
            }
            if (lobbyUI.addButton("Year 4"))
            {
                changeSchoolYear(4);
            }
            if (lobbyUI.addButton("Year 5"))
            {
                changeSchoolYear(5);
            }
            if (lobbyUI.addButton("Year 6"))
            {
                changeSchoolYear(6);
            }
            if (lobbyUI.addButton("Year 7"))
            {
                changeSchoolYear(7);
            }
        }

        schoolYear = "School year: " + std::to_string(lobbyState.schoolYear);
        lobbyUI.addTitle(schoolYear);

        if (lobbyUI.addButton("Toggle Ready"))
        {
            bool myReady = false;

            auto it = lobbyState.players.find(playerID);
            if (it != lobbyState.players.end())
            {
                myReady = it->second.ready;
            }
            if (myReady)
            {
                LobbyActionPacket unready;
                unready.actionType = LobbyActionType::PlayerUnready;
                unready.playerId = playerID;

                ENetPacket* packet = enet_packet_create(&unready, sizeof(LobbyActionPacket), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
                readyString = "Ready status: Not ready";
            }
            else
            {
                LobbyActionPacket ready;
                ready.actionType = LobbyActionType::PlayerReady;
                ready.playerId = playerID;

                ENetPacket* packet = enet_packet_create(&ready, sizeof(LobbyActionPacket), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
                readyString = "Ready status: Ready";
            }
        }

        lobbyUI.addTitle("Select character");
        if (lobbyUI.addButton("Harry"))
        {
            setCharacter(CharacterID::Harry);
        }
        if (lobbyUI.addButton("Ron"))
        {
            setCharacter(CharacterID::Ron);
        }
        if (lobbyUI.addButton("Hermione"))
        {
            setCharacter(CharacterID::Hermione);
        }
        if (lobbyUI.addButton("Neville"))
        {
            setCharacter(CharacterID::Neville);
        }
        if (lobbyUI.addButton("Luna"))
        {
            setCharacter(CharacterID::Luna);
        }

        auto it = lobbyState.players.find(playerID);
        if (it != lobbyState.players.end())
        {
            character = getCharacterName(it->second.charID);
        }

        lobbyUI.addTitle("Selected: " + character);

        lobbyUI.addTitle(readyString);

        if (isHost)
        {
            if (lobbyUI.addButton("Start Game"))
            {
                LobbyActionPacket startGame;
                startGame.actionType = LobbyActionType::StartGame;
                ENetPacket* packet = enet_packet_create(&startGame, sizeof(LobbyActionPacket), ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(peer, 0, packet);
            }
        }

        if (lobbyUI.addButton("Quit"))
        {
            //TODO: Disconnect
        }

        lobbyUI.updateAndRenderWidgets();
        lobbyUI.lastFrameWidgets = lobbyUI.widgets;
        break;

    }
    default:
        break;
    }
    
}
void Client::changeSchoolYear(int year)
{
    std::cout << "Attempting to change school year to " << year << "\n";

    sendLobbyAction(playerID, LobbyActionType::SetSchoolYear, year);
}

void Client::setCharacter(CharacterID charID)
{
    std::cout << playerID << " changing character to: " << getCharacterName(charID)<< "\n";
    sendLobbyAction(playerID, LobbyActionType::SelectCharacter, -1, charID);
}

