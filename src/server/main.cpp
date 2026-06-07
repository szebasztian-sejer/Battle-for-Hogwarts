#include <server.h>
#include <thread>
#include <chrono>

int main ()
{
	Server server;
	server.init();

	while (true)
	{
		if (!server.update())
		{
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	server.close();
}