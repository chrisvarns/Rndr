#include "Utils.h"

#include <sdl/SDL.h>

size_t ShaderUtils::LoadShaderBinary(std::string filename, void** data)
{
	SDL_RWops* psFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\" + filename).c_str(), "rb");
	if (psFile == nullptr)
	{
		SDL_Log("SDL_RWFromFile failed");
		return -1;
	}
	size_t psDataSize = SDL_RWsize(psFile);
	*data = malloc(psDataSize);
	SDL_RWread(psFile, *data, psDataSize, 1);
	SDL_RWclose(psFile);

	return psDataSize;
}