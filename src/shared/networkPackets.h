#pragma once
#include <cstdint>
#include <lobbyState.h>
#include <gameState.h>
#include <characters.h>

constexpr int MAX_PLAYERS = 4;

enum class PacketType : uint8_t
{
    None,
    CreateLobby,
    LobbyAction,
    PlayerAction,
    LobbyState,
    AssignPlayerID,
    GameState,
    ChatMessage,
    PACKET_TYPE_COUNT,
};

enum class LobbyActionType : uint8_t
{
    None,
    PlayerReady,
    PlayerUnready,
    SetSchoolYear,
    SelectCharacter,
    StartGame,
    LOBBY_ACTION_COUNT,
};

enum class GameActionType : uint8_t
{
    None,
    PlayCard,
    BuyCard,
    UseAttackToken,
    DiscardCard,
    HealPlayer,
    EndTurn,
    GAME_ACTION_COUNT,
};

struct PlayerActionPacket
{
    PacketType packetType = PacketType::PlayerAction;

    uint32_t playerId = 0;
    GameActionType actionType = GameActionType::None;

    uint32_t cardInstanceId = 0;
    uint32_t targets = 0; //1st bit player1, 2nd bit player2, 3rd bit player3, 4th bit player4
                          //5th bit Villain1, 6th bit Villain2, 7th bit Villain3, 8th bit Villain4
};

struct LobbyActionPacket
{
    PacketType packetType = PacketType::LobbyAction;
    uint32_t playerId = 0;
    LobbyActionType actionType = LobbyActionType::None;
    int schoolYear = 1;
    CharacterID charID = CharacterID::None;
};

struct CreateLobbyPacket
{
    PacketType packetType = PacketType::CreateLobby;
};

struct LobbyStatePacket
{
    PacketType packetType = PacketType::LobbyState;
    int schoolYear = 1;
    int playerCount = 0;
    LobbyPlayer players[MAX_PLAYERS] = {};

};

struct GameStatePacket
{
    PacketType packetType = PacketType::GameState;
    GameState gameState = {};

};

struct AssignPlayerIDPacket
{
    PacketType packetType = PacketType::AssignPlayerID;
    int playerID = 0;
};