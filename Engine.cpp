#include "Engine.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <string>
#include <sstream>

using namespace std;

vector<string> Engine::ms_Commands = { "xres", "yres", "scene" };

int Engine::ParseArgs()
{
	for (int i = 1; i < m_NumCmdLineArgs; ++i)
	{
		string cmd = m_CmdLineArgs[i];

		//split the string
		size_t mid = cmd.find("=");
		string arg = cmd.c_str() + mid + 1;
		cmd.resize(mid);

		if (std::find(ms_Commands.begin(), ms_Commands.end(), cmd) == ms_Commands.end())
			return 1;

		if (cmd == "xres")
		{
			m_WindowWidth = stoi(arg);
		}
		else if (cmd == "yres")
		{
			m_WindowHeight = stoi(arg);
		}
	}
	return 0;
}

Engine::Engine(int argc, char** argv)
	: m_NumCmdLineArgs(argc)
	, m_CmdLineArgs(argv)
	, m_WindowWidth(1280)
	, m_WindowHeight(720)
{
}

int Engine::Init()
{
	if (ParseArgs())
	{
		SDL_Log("Failed to parse args");
		return 1;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("Unable to init Video: %s", SDL_GetError());
		return 2;
	}

	m_SdlWindow = SDL_CreateWindow(
		"Rndr",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		m_WindowWidth, m_WindowHeight, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

	if (!m_SdlWindow) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		return 3;
	}

	return 0;
}

Engine::~Engine()
{
	if (m_SdlWindow)		SDL_DestroyWindow(m_SdlWindow);
}

int Engine::Execute()
{
	return 0;
}