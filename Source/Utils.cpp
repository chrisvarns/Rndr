#include "Utils.h"

#include <vector>
#include <sdl/SDL.h>

std::vector<char> Utils::LoadFile(const std::string& filename)
{
    std::vector<char> ret(0);
	SDL_RWops* psFile = SDL_RWFromFile((std::string(SDL_GetBasePath()) + "\\" + filename).c_str(), "rb");
	if (psFile == nullptr)
	{
		SDL_Log("SDL_RWFromFile failed");
		return ret;
	}
	size_t psDataSize = SDL_RWsize(psFile);
    ret.resize(psDataSize);
	SDL_RWread(psFile, ret.data(), psDataSize, 1);
	SDL_RWclose(psFile);

	return ret;
}