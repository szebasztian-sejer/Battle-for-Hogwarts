#include <gameMain.h>
#include <gamePlay.h>
#include <widget.h>
#include <assetManager.h>
#include <raylib.h>
#include <iostream>
#include <server.h>
#include <client.h>

AssetManager assetManager;
GamePlay gamePlay;
Server server;
Client client;

Widget mainMenu;
Widget hostingMenu;
Widget hostQuit;
Widget joinMenu;

Widget testHost;
Widget testPeer;

bool serverInitialized = false;
bool clientConnected = false;
bool clientInitialized = false;

std::string ipAddress = "";

bool initGame()
{
	assetManager.loadAll();
	gamePlay.gameState = GamePlay::GameState::MAIN_MENU;
	return true;
}

bool updateGame()
{
	
	Vector2 mouse = GetMousePosition();
	switch (gamePlay.gameState)
	{
		case GamePlay::GameState::MAIN_MENU:
		{
			
			mainMenu.drawRec = { 0,0,800,600 };
			mainMenu.addTitle("Battle For Hogwarts");

			if (mainMenu.addButton("Host game"))
			{
				gamePlay.gameState = GamePlay::GameState::HOSTING;
			}

			if (mainMenu.addButton("Join game"))
			{
				gamePlay.gameState = GamePlay::GameState::JOINING;
			}

			if (mainMenu.addButton("Quit"))
			{
				gamePlay.gameState = GamePlay::GameState::CLOSING;
			}

			mainMenu.updateAndRenderWidgets();
			mainMenu.lastFrameWidgets = mainMenu.widgets;
			mainMenu.widgets.clear();
			return true;
		}



		case GamePlay::GameState::HOSTING:
		{
			if (!serverInitialized)
			{
				if (!server.init()) { return false; }
				serverInitialized = true;
			}

			if (!clientConnected)
			{
				if (!client.init()) { return false; }
				if (!client.connect("127.0.0.1")) { return false; }
				clientConnected = true;
			}
			server.update();
			client.update();

			
			hostingMenu.drawRec = { 0,0,800,600 };
			hostingMenu.addTitle("Waiting for clients to connect...");
			if (hostingMenu.addButton("Start game"))
			{
				gamePlay.gameState = GamePlay::GameState::IN_GAME_HOST;
			}
			
			hostQuit.drawRec = { 400,400,200,200 };
			if (hostQuit.addButton("Quit"))
			{
				gamePlay.gameState = GamePlay::GameState::CLOSING;
			}

			hostQuit.updateAndRenderWidgets();
			hostQuit.lastFrameWidgets = hostQuit.widgets;
			hostQuit.widgets.clear();

			hostingMenu.updateAndRenderWidgets();
			hostingMenu.lastFrameWidgets = hostingMenu.widgets;
			hostingMenu.widgets.clear();

			return true;
			
		}

		case GamePlay::GameState::JOINING:
		{
			if (!clientInitialized)
			{
				if (!client.init()) { return false; }
				clientInitialized = true;
			}
			
			joinMenu.drawRec = { 0,0,800,600 };
			joinMenu.addTitle("Type an IP address to connect to!");
			ipAddress = joinMenu.addTextBox(ipAddress);
			if (joinMenu.addButton("Join") && !clientConnected)
			{
				if (!client.connect(ipAddress.c_str()))
				{
					std::cout << "Failed to connect to server.";
					gamePlay.gameState = GamePlay::GameState::CLOSING;
				}
				clientConnected = true;
				gamePlay.gameState = GamePlay::GameState::IN_GAME_PEER;
			}

			joinMenu.updateAndRenderWidgets();

			if (joinMenu.widgets.size() > 1 &&
				joinMenu.widgets[1].type == WidgetTypes::TextBox)
			{
				ipAddress = joinMenu.widgets[1].text;
			}

			joinMenu.lastFrameWidgets = joinMenu.widgets;
			joinMenu.widgets.clear();

			return true;

		}

		case GamePlay::GameState::IN_GAME_HOST:
		{
			server.update();
			testHost.drawRec = { 0,0,800,600 };
			if (testHost.addButton("Quit"))
			{
				std::cout << "Successful hosting, quitting...";
				gamePlay.gameState = GamePlay::GameState::CLOSING;
			}
			testHost.updateAndRenderWidgets();
			testHost.lastFrameWidgets = testHost.widgets;
			testHost.widgets.clear();
			return true;
		}

		case GamePlay::GameState::IN_GAME_PEER:
		{
			client.update();
			
			testPeer.drawRec = { 0,0,800,600 };
			if (testPeer.addButton("Quit"))
			{
				std::cout << "Successfully joined lobby, quitting...";
				gamePlay.gameState = GamePlay::GameState::CLOSING;
			}
			testPeer.updateAndRenderWidgets();
			testPeer.lastFrameWidgets = testPeer.widgets;
			testPeer.widgets.clear();
			return true;
		}

		case GamePlay::GameState::CLOSING:
		{
			std::cout << "Game is quitting\n";
			return false;
		}
	}
	return true;
}

void closeGame()
{
	if (clientInitialized)
	{
		client.close();
	}

	if (serverInitialized)
	{
		server.close();
	}
}