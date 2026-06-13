#include <raylib.h>
#include <client.h>
#include <iostream>
#include <chrono>
#include <thread>

int main()
{
	Client client;

	client.init();

	bool running = true;

	while (running && !WindowShouldClose())
	{
		if (!client.updateNetwork())
		{
			running = false;
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);

		client.updateUI();

		EndDrawing();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	CloseWindow();
	CloseAudioDevice();
	client.close();

	return 0;
}