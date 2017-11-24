#pragma once

#include <string>
#include <vector>

#include "CPUTexture.h"

namespace FileUtils
{
std::string Combine(const std::string& path1, const std::string& path2);
std::string GetProcessWorkingDir();
std::string GetParentDirectory(const std::string& path);

std::vector<char> LoadFile(const std::string& filename);
std::vector<char> LoadFileAbsolute(const std::string& absPath);
CPUTexture LoadUncompressedTGA(const std::string& filename);
}