#include "TextureMap.h"

#include "Engine.h"
#include "FileUtils.h"
#include "D3D11/D3D11RHI.h"

ID3D11Texture2D* TextureMap::GetTexture2DFromPath(const std::string& path)
{
    auto iter = map.find(path);
    if (iter != map.end()) return iter->second;

    auto cpuTexture = FileUtils::LoadUncompressedTGA(path);
    auto rhiHandle = g_Engine->rhi.CreateTexture2D(cpuTexture);
    map.insert({ path, rhiHandle });
    return rhiHandle;
}