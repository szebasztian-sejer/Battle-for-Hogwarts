#include <server.h>
#include <client.h>
#include <iostream>
#include <gameMain.h>
#include <raylib.h>

int main()
{
#if 0
	Server server;
	server.init();
	Client client;
	client.init();
	client.connect("127.0.0.1");

	while (true)
	{
		for (int i = 0; i < 1000000; i++)
		{
			if (i % 50000 == 0)
			{
				client.SendPlayerAction(
					1,
					ActionType::PlayCard,
					42,
					0
				);
			}
			server.update();
			client.update();
		}

		std::cout << "Test successful!\n";
		break;
	}

	client.close();
	server.close();
#endif

	if (!initGame())
	{
		return EXIT_FAILURE;
	}

	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1024, 860, "Battle for Hogwarts");

	InitAudioDevice();
	SetExitKey(KEY_NULL);
	SetTargetFPS(240);


	bool running = true;

	while (running && !WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(RAYWHITE);

		if (!updateGame())
		{
			running = false;
		}

		EndDrawing();
	}

	CloseWindow();
	CloseAudioDevice();


	return 0;

}