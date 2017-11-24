#include <cassert>
#include <filesystem>
#include <vector>
#include <direct.h>
#include <sdl/SDL.h>

#include "FileUtils.h"

std::string FileUtils::Combine(const std::string& path1, const std::string& path2)
{
    std::experimental::filesystem::path left(path1);
    std::experimental::filesystem::path right(path2);
    return (left / right).string();
}

std::string FileUtils::GetProcessWorkingDir()
{
    char* cwd = _getcwd(0, 0);
    std::string ret(cwd);
    std::free(cwd);
    return ret;
}

std::string FileUtils::GetParentDirectory(const std::string& path)
{
    std::experimental::filesystem::path stdPath = path;
    return stdPath.parent_path().string();
}

std::vector<char> FileUtils::LoadFile(const std::string& filename)
{
    auto fullpath = std::string(SDL_GetBasePath()) + filename;
    return LoadFileAbsolute(fullpath);
}

std::vector<char> FileUtils::LoadFileAbsolute(const std::string& absPath)
{
    SDL_RWops* psFile;
    psFile = SDL_RWFromFile(absPath.c_str(), "rb");

    assert(psFile);
	size_t psDataSize = SDL_RWsize(psFile);
    std::vector<char> ret(0);
    ret.resize(psDataSize);
	SDL_RWread(psFile, ret.data(), psDataSize, 1);
	SDL_RWclose(psFile);

	return ret;
}

CPUTexture FileUtils::LoadUncompressedTGA(const std::string& absolutePath)
{
    auto fileData = LoadFileAbsolute(absolutePath);
    assert(fileData[2] == 2); // uncompressed RGB/RGBA
    CPUTexture texture;

    texture.width = *(reinterpret_cast<uint16_t*>(fileData.data() + 12));
    texture.height = *(reinterpret_cast<uint16_t*>(fileData.data() + 14));
    texture.colorChannels = *(fileData.data() + 16) / 8;
    assert(texture.colorChannels == 3 || texture.colorChannels == 4);

    auto imageDataLength = texture.width * texture.height * texture.colorChannels;
    auto imageData = fileData.begin() + 18;
    texture.data = std::vector<char>(imageData, imageData + imageDataLength);

    return texture;
}