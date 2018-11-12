#pragma once
#include <string>
#include <map>

struct ID3D11Texture2D;

class TextureMap
{
public:
    ID3D11Texture2D* GetTexture2DFromPath(const std::string& path);

private:
    std::map<std::string, ID3D11Texture2D*> map;
};