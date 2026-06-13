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
        case ClientState::Menu:
        {
            handleMenuEvent(enetEvent);
            break;
        }

        case ClientState::WaitingRoom:
        {
            handleWaitingRoomEvent(enetEvent);
            break;
        }

        case ClientState::InGame:
        {
            handleGameEvent(enetEvent);
            break;
        }

        default:
            break;
        }
    }

    return clientState != ClientState::Closing;
}

void Client::updateUI()
{
    displayAndInteract();
}

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

            HostLobbyPacket hostLobbyPacket;
            ENetPacket* packet = enet_packet_create(
                &hostLobbyPacket,
                sizeof(HostLobbyPacket),
                ENET_PACKET_FLAG_RELIABLE
            );

            enet_peer_send(peer, 0, packet);
            enet_host_flush(client);
        }
        else
        {
            std::cout << "Creaing join request packet...\n";

            JoinLobbyPacket joinLobbyPacket;
            joinLobbyPacket.lobbyID = static_cast<uint32_t>(std::stoi(lobbyID));
            std::cout << "Static cast succeeded\n";
            ENetPacket* packet = enet_packet_create(
                &joinLobbyPacket,
                sizeof(JoinLobbyPacket),
                ENET_PACKET_FLAG_RELIABLE
            );

            std::cout << "Sending join lobby request: " << lobbyID << "\n";

            enet_peer_send(peer, 0, packet);
            std::cout << "Join request sent\n";
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
    case PacketType::InviteToLobby:
    {
        std::cout << "Received lobby invitation\n";
        InviteToLobbyPacket invitationPacket;
        std::memcpy(&invitationPacket, packet->data, sizeof(InviteToLobbyPacket));

        player.isHost = invitationPacket.isHost;
        clientState = ClientState::WaitingRoom;
        return;
    }
    case PacketType::AssignPlayerID:
    {
        AssignPlayerIDPacket idpacket;
        std::memcpy(&idpacket, packet->data, sizeof(AssignPlayerIDPacket));
        playerID = idpacket.playerID;
        player.playerID = idpacket.playerID;

        std::cout << "Received player ID: "<<player.playerID<<"\n";
        break;
    }
    default:
        std::cout << "Received invalid packet\n";
        break;
    }

   
    
}

void Client::handleWaitingRoomEvent(ENetEvent& enetEvent)
{
    switch (enetEvent.type)
        {
        case ENET_EVENT_TYPE_RECEIVE:
        {
            handleWaitingRoomPacket(enetEvent.peer, enetEvent.packet);
            enet_packet_destroy(enetEvent.packet);
            break;
        }
        case ENET_EVENT_TYPE_DISCONNECT:
        {
   
  
            std::cout << "Disconnected. data = " << enetEvent.data << "\n";
            clientState = ClientState::Menu;
            connectionStarted = false;
            connectedToServer = false;
            wantsToCreateLobby = false;
            break;
        }
        default:
            break;
    }
}

void Client::handleWaitingRoomPacket(ENetPeer* peer, ENetPacket* packet)
{
    PacketType packetType;
    std::memcpy(&packetType, packet->data, sizeof(PacketType));

    switch (packetType)
    {
    case PacketType::GameStart:
    {
        clientState = ClientState::InGame;
        break;
    }
    case PacketType::WaitingRoomState:
    {
        std::cout << "Received waiting room state\n";
        WaitingRoomStatePacket statePacket;
        std::memcpy(&statePacket, packet->data, sizeof(WaitingRoomStatePacket));

        lobby.lobbyId = statePacket.lobbyID;
        std::cout << "setting lobby ID to: "<< lobby.lobbyId<<"\n";
        lobby.schoolYear = statePacket.schoolYear;
        lobby.players.clear();

        for (int i = 0; i < statePacket.playerCount; ++i)
        {
            auto packetPlayer = statePacket.players[i];
            LobbyPlayer lp;
            lp.playerID = packetPlayer.playerID;
            lp.ready = packetPlayer.ready;
            lp.isHost = packetPlayer.isHost;
            lp.charID = packetPlayer.charID;

            lobby.players[lp.playerID] = lp;

            if (packetPlayer.playerID == player.playerID)
            {
                player.ready = packetPlayer.ready;
                player.isHost = packetPlayer.isHost;
                player.charID = packetPlayer.charID;
            }
        }

        break;
    }
    default:
        break;
    }

}

void Client::sendWaitingRoomAction(
    WaitingRoomActionType waitingRoomAction, 
    int schoolYear, 
    CharacterID charID)
{
    WaitingRoomActionPacket waitingRoomActionPacket;
    waitingRoomActionPacket.lobbyID = lobby.lobbyId;
    waitingRoomActionPacket.waitingRoomActionType = waitingRoomAction;
    waitingRoomActionPacket.playerID = playerID;
    waitingRoomActionPacket.newYear = schoolYear;
    waitingRoomActionPacket.newCharID = charID;

    ENetPacket* packet = enet_packet_create(
        &waitingRoomActionPacket,
        sizeof(WaitingRoomActionPacket),
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
    case ClientState::Menu:
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

        mainMenu.addTitle("IP address:");
        mainMenu.addTextBox(ipAddress);
        mainMenu.addTitle("Lobby ID:");
        mainMenu.addTextBox(lobbyID);
        if (mainMenu.addButton("Connect"))
        {
            wantsToCreateLobby = false;

            if (!connectionStarted)
            {
                connectionStarted = connect(ipAddress.c_str());
            }
        }

        mainMenu.updateAndRenderWidgets();

        if (mainMenu.widgets.size() > 4)
        {
            if (mainMenu.widgets[2].type == WidgetTypes::TextBox)
            {
                ipAddress = mainMenu.widgets[2].text;
            }

            if (mainMenu.widgets[4].type == WidgetTypes::TextBox)
            {
                lobbyID = mainMenu.widgets[4].text;
            }
        }

        mainMenu.lastFrameWidgets = mainMenu.widgets;

        break;
    }
    case ClientState::WaitingRoom:
    {
        lobbyUI.widgets.clear();
        lobbyUI.addTitle("Lobby ID: " + std::to_string(lobby.lobbyId));
        if (player.isHost)
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

        schoolYear = "School year: " + std::to_string(lobby.schoolYear);
        lobbyUI.addTitle(schoolYear);

        if (lobbyUI.addButton("Toggle Ready"))
        {
            sendWaitingRoomAction(WaitingRoomActionType::ToggleReady);
            readyString = player.ready ? "Ready status: Not ready" : "Ready status: Ready";
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

        character = getCharacterName(player.charID);
 
        lobbyUI.addTitle("Selected: " + character);

        lobbyUI.addTitle(readyString);

        if (player.isHost)
        {
            if (lobbyUI.addButton("Start Game"))
            {
                sendWaitingRoomAction(WaitingRoomActionType::StartGame);
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

    sendWaitingRoomAction(WaitingRoomActionType::SetYear, year);
}

void Client::setCharacter(CharacterID charID)
{
    std::cout << playerID << " changing character to: " << getCharacterName(charID)<< "\n";
    sendWaitingRoomAction(WaitingRoomActionType::SetCharacter, -1, charID);
}

