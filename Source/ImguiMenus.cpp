#include "ImguiMenus.h"

#include "imgui.h"

namespace ImGui::Integration
{

static bool g_RenderCameraControls = false;

extern float g_Controls_Camera_Speed = 0.001f;
extern float g_Controls_Camera_Sensitivity = 0.001f;

void RenderMenus()
{
	RenderMainMenu();

	if(g_RenderCameraControls) RenderCameraControlMenu(&g_RenderCameraControls);
}

void RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Controls"))
		{
			ImGui::MenuItem("Camera", NULL, &g_RenderCameraControls);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void RenderCameraControlMenu(bool* pOpen)
{
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiSetCond_FirstUseEver);

	if (ImGui::Begin("Controls: Camera", pOpen))
	{
		ImGui::SliderFloat("Speed", &g_Controls_Camera_Speed, 0.0f, 0.05f);
		ImGui::SliderFloat("Sensitivity", &g_Controls_Camera_Sensitivity, 0.0f, 0.01f);
	}

	ImGui::End();
}

} // namespace ImGui::Integration