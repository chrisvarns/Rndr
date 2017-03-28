#pragma once

#include <string>

namespace ShaderUtils
{
// Returns the size of the shader data in bytes
size_t LoadShaderBinary(std::string filename, void** data);
}