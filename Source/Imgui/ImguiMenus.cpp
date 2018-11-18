#include "ImguiMenus.h"

#include "imgui.h"

namespace ImGui::Integration
{

float g_Controls_Camera_Speed = 0.01f;
float g_Controls_Camera_Sensitivity = 0.001f;

void RenderCameraMenu(bool* pOpen)
{
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiSetCond_FirstUseEver);

	if (ImGui::Begin("Camera", pOpen))
	{
		ImGui::SliderFloat("Speed", &g_Controls_Camera_Speed, 0.0f, 0.05f);
		ImGui::SliderFloat("Sensitivity", &g_Controls_Camera_Sensitivity, 0.0f, 0.01f);
		ImGui::End();
	}

}

float g_Lighting_Ambient[3] = { 0.2f, 0.2f, 0.2f };

void RenderLightingWindow(bool* pOpen) {
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiSetCond_FirstUseEver);

	if (ImGui::Begin("Lighting", pOpen))
	{
		ImGui::ColorEdit3("Ambient Color", g_Lighting_Ambient);
		ImGui::End();
	}

}

static bool g_cameraWindowOpen = false;
static bool g_lightingWindowOpen = false;

void RenderMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Controls"))
		{
			ImGui::MenuItem("Camera", NULL, &g_cameraWindowOpen);
			ImGui::MenuItem("Lighting", NULL, &g_lightingWindowOpen);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void RenderMenus()
{
	RenderMainMenu();

	if(g_cameraWindowOpen) RenderCameraMenu(&g_cameraWindowOpen);
	if (g_lightingWindowOpen) RenderLightingWindow(&g_lightingWindowOpen);

}

} // namespace ImGui::Integration