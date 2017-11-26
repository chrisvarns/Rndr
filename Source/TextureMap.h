#pragma once
#include <string>
#include <map>

#include "RHI/RHI.h"

class TextureMap
{
public:
    RHI::RHITexture2DHandle GetTexture2DFromPath(const std::string& path);

private:
    std::map<std::string, RHI::RHITexture2DHandle> map;
};