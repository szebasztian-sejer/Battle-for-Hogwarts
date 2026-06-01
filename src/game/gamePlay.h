#pragma once

struct GamePlay
{
	enum class GameState
	{
		MAIN_MENU,
		HOSTING,
		JOINING,
		IN_GAME_HOST,
		IN_GAME_PEER,
		CLOSING,
		GAME_STATE_COUNT,
	};

	GameState gameState = GameState::MAIN_MENU;
};