#include "Engine.h"

#include "stdio.h"

int main(int argc, char** argv)
{
	Engine* engine = new Engine(argc, argv);
	if (engine->Init()) return 1;

	while (true)
	{
		engine->Execute();

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
		}
	}
}