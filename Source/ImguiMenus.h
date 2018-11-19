#pragma once
#include "glm/glm.hpp"

namespace ImGui::Integration
{
	void RenderMenus();

	// Controls
	extern float g_Controls_Camera_Speed;
	extern float g_Controls_Camera_Sensitivity;

	// Lighting
	extern glm::vec3 g_Lighting_AmbientCol;
	extern glm::vec3 g_Lighting_DirectionalCol;
	extern glm::vec3 g_Lighting_DirectionalRot;
}