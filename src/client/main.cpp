#include <raylib.h>
#include <client.h>
#include <iostream>

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
	}

	CloseWindow();
	CloseAudioDevice();
	client.close();

	return 0;
}