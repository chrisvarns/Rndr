#pragma once
namespace ImGui::Integration
{
	void RenderMenus();

	// Controls
	extern float g_Controls_Camera_Speed;
	extern float g_Controls_Camera_Sensitivity;

	// Lighting
	extern float g_Lighting_Ambient[3];
}