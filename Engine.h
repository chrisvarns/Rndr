#pragma once

#include <string>
#include <vector>
#include <SDL.h>

using namespace std;

class Engine
{
public:
	Engine(int argc, char** argv);
	~Engine();
	int Init();
	int Execute();

private:
	int		m_NumCmdLineArgs;
	char**	m_CmdLineArgs;
	int		m_WindowWidth;
	int		m_WindowHeight;

	SDL_Window*		m_SdlWindow;
	SDL_GLContext	m_SdlGlContext;

	/// Statics
private:
	static vector<string> ms_Commands;
	int ParseArgs();

};

