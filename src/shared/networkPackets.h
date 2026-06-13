#pragma once
#include <cstdint>
#include <characters.h>
#include <lobbyPlayer.h>

constexpr int MAX_PLAYERS = 4;

enum class PacketType : uint8_t
{
	None,
	AssignPlayerID,
	HostLobby,
	JoinLobby,
	InviteToLobby,
	WaitingRoomState,
	WaitingRoomAction,
	GameStart,
	InGameState,
	InGameAction,
	PACKET_TYPE_COUNT,
};

struct AssignPlayerIDPacket
{
	PacketType packetType = PacketType::AssignPlayerID;
	int playerID = 0;
};

struct HostLobbyPacket
{
	PacketType packetType = PacketType::HostLobby;
};

struct JoinLobbyPacket
{
	PacketType packetType = PacketType::JoinLobby;
	uint32_t lobbyID = 0;
};

struct InviteToLobbyPacket
{
	PacketType packetType = PacketType::InviteToLobby;
	uint32_t lobbyID = 0;
	uint32_t playerID = 0;
	bool isHost = false;
};


struct WaitingRoomStatePacket
{
	PacketType packetType = PacketType::WaitingRoomState;
	uint32_t lobbyID = 0;
	int schoolYear = 1;
	int playerCount = 0;
	LobbyPlayer players[MAX_PLAYERS] = {};
	
};

enum class WaitingRoomActionType
{
	None,
	SetYear,
	SetCharacter,
	ToggleReady,
	StartGame,
	WAITING_ROOM_ACTION_TYPE_COUNT

};

struct WaitingRoomActionPacket
{
	PacketType packetType = PacketType::WaitingRoomAction;
	uint32_t lobbyID = 0;
	uint32_t playerID = 0;
	WaitingRoomActionType waitingRoomActionType = WaitingRoomActionType::None;
	uint32_t newYear = 1;
	CharacterID newCharID = CharacterID::None;
};