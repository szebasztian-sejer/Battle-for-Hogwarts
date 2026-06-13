#pragma once
#include <widget.h>
#include <string>
#include <networkPackets.h>
#include <gameState.h>
#include <lobby.h>


struct _ENetHost;
struct _ENetPeer;
struct _ENetEvent;
struct _ENetPacket;

using ENetHost = _ENetHost;
using ENetPeer = _ENetPeer;
using ENetEvent = _ENetEvent;
using ENetPacket = _ENetPacket;

class Client
{
public:
    ENetHost* client = nullptr;
    ENetPeer* peer = nullptr;

    enum class ClientState
    {
        None,
        Menu,
        WaitingRoom,
        InGame,
        Closing,
        CLIENT_STATE_COUNT,
    };

    ClientState clientState = ClientState::Menu;
    GameState gameState = {};
    LobbyPlayer player = {};
    Lobby lobby;
    uint32_t playerID = 0;
    Widget mainMenu = {};
    Widget lobbyUI = {};

    bool connectionStarted = false;
    bool connectedToServer = false;
    bool wantsToCreateLobby = false;

    std::string ipAddress = "";
    std::string lobbyID = "";
    std::string schoolYear = "";
    std::string readyString = "Not ready";
    std::string character = "None";
    bool ready = false;
    bool isHost = false;
    

    Client();
    bool init();
    bool connect(const char* serverAddress);
    bool updateNetwork();
    void updateUI();

    void handleMenuEvent(ENetEvent& enetEvent);
    void handleWaitingRoomEvent(ENetEvent& enetEvent);
    void handleGameEvent(ENetEvent& enetEvent);

    //void handleGameDisconnect(ENetPeer* peer);
    //void handleDisconnect(ENetPeer* peer);

    void handleMenuPacket(ENetPeer* peer, ENetPacket* packet);
    void handleWaitingRoomPacket(ENetPeer* peer, ENetPacket* packet);
    void handleGamePacket(ENetPeer* peer, ENetPacket* packet);

    void close();

    void displayAndInteract();
    void sendWaitingRoomAction(WaitingRoomActionType waitingRoomAction, int schoolYear = -1, CharacterID charID = CharacterID::None);
    void changeSchoolYear(int year);
    void setCharacter(CharacterID charID);

    ~Client();
};