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
    auto colorChannels = *(fileData.data() + 16) / 8;
    assert(colorChannels == 3 || colorChannels == 4);

    auto srcImageDataLength = texture.width * texture.height * colorChannels;
    auto srcImageDataBegin = fileData.data() + 18;
    auto srcImageDataEnd = srcImageDataBegin + srcImageDataLength;

    if (colorChannels == 4)
    {
        // Simple copy
        texture.data = std::vector<char>(srcImageDataBegin, srcImageDataEnd);
    }
    else
    {
        // Must add the alpha channel manually
        auto destImageDataLength = texture.width * texture.height * 4;
        texture.data.resize(destImageDataLength);

        auto destItr = texture.data.data();
        auto srcItr = srcImageDataBegin;
        do {
            *(destItr+0) = *srcItr+0;
            *(destItr+1) = *srcItr+1;
            *(destItr+2) = *srcItr+2;
            *(destItr+3) = 255;
            destItr += 4;
            srcItr += 3;
        } while (destImageDataLength -= 4);
    }

    return texture;
}