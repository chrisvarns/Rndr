#pragma once
#include "glm/glm.hpp"

namespace Globals
{
	// Controls
	extern float CameraSpeed;
	extern float CameraSensitivity;

	// Lighting
	extern glm::vec3 LightingAmbientColor;
	extern glm::vec3 LightingDirectionalColor;
	extern glm::vec3 LightingDirectionalRot;
}

namespace ImGui::Integration
{
	void RenderMenus();
}