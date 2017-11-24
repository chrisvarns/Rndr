#pragma once

#include <vector>

class CPUTexture
{
public:
    int width, height;
    int colorChannels;
    std::vector<char> data;
};