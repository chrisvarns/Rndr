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

	// Initial context, so we can query the server.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	m_SdlGlContext = SDL_GL_CreateContext(m_SdlWindow);

	if (!m_SdlGlContext) {
		SDL_Log("Unable to create GL context: %s", SDL_GetError());
		return 3;
	}

	SDL_GL_SetSwapInterval(1);

	return 0;
}

Engine::~Engine()
{
	if (m_SdlGlContext)		SDL_GL_DeleteContext(m_SdlGlContext);
	if (m_SdlWindow)		SDL_DestroyWindow(m_SdlWindow);
}

int Engine::Execute()
{
	int loopCounter = 0;

	char indices[] = {
		0, 1, 2
	};

	float vertPos[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.1f,
	};

	float vertCol[] = {
		1.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 1.0f,
	};
	
	string vShader =
		"vec4 vertPos;"

		"main() {"
		"	gl_Position = vertPos;"
		"}";

	string fShader =
		"vec4 vertCol;"
		"void main() {"
		"	gl_FragColor = vertCol;"
		"}";

	while (true)
	{
		glClearColor((float)200 / 255, (float)162 / 255, (float)200 / 255, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(m_SdlWindow);
		SDL_Delay(1000);
		glClearColor((float)64 / 255, (float)224 / 255, (float)208 / 255, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(m_SdlWindow);
		SDL_Delay(1000);
		//glGenBuffers();
		//glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE, indices);
	}
	return 0;
}