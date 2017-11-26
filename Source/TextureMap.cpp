#include "TextureMap.h"

#include "Engine.h"
#include "FileUtils.h"

RHI::RHITexture2DHandle TextureMap::GetTexture2DFromPath(const std::string& path)
{
    auto iter = map.find(path);
    if (iter != map.end()) return iter->second;

    auto cpuTexture = FileUtils::LoadUncompressedTGA(path);
    auto rhiHandle = Engine::g_Engine->rhi->CreateTexture2D(cpuTexture);
    map.insert({ path, rhiHandle });
    return rhiHandle;
}