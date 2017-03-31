#pragma once
namespace ImGui::Integration
{
	void RenderMenus();
	void RenderMainMenu();
	void RenderCameraControlMenu(bool* pOpen);

	// Controls
	extern float g_Controls_Camera_Speed;
	extern float g_Controls_Camera_Sensitivity;
}