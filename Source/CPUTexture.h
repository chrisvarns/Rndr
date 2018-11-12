#pragma once

#include <vector>

struct CPUTexture
{
	int width, height;
	std::vector<char> data;
};