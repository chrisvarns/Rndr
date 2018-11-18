#include "ImguiMenus.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"

#include "imgui/imgui.h"
#include "imgui/ImGuizmo.h"
#include "Engine.h"

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
	}
	ImGui::End();
}

float g_Lighting_AmbientCol[3] = { 0.2f, 0.2f, 0.2f };
float g_Lighting_DirectionalCol[3] = { 0.8f, 0.8f, 0.8f };
glm::vec3 g_Lighting_DirectionalRot = glm::vec3(0.f);

bool showLightRotationWidget = false;


void RenderLightingWindow(bool* pOpen) {
	ImGui::SetNextWindowSize(ImVec2(350, 560), ImGuiSetCond_FirstUseEver);

	if (ImGui::Begin("Lighting", pOpen))
	{
		ImGui::Text("Ambient");
		ImGui::ColorEdit3("Ambient Color", g_Lighting_AmbientCol);

		ImGui::Separator();

		ImGui::Text("Directional Light");
		ImGui::ColorEdit3("Directional Color", g_Lighting_DirectionalCol);
		if (ImGui::Button("Directional Dir")) {
			showLightRotationWidget ^= true;
		}
		if (showLightRotationWidget) {
			ImGuiIO& io = ImGui::GetIO();
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

			ImGuizmo::Enable(!SDL_GetRelativeMouseMode());

			float mat[16];
			auto translate = g_Engine->camera.viewPos + (g_Engine->camera.viewDir * 0.5f) + (g_Engine->camera.rightDir * 0.3f);
			glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
			ImGuizmo::RecomposeMatrixFromComponents((float*)&translate, (float*)&g_Lighting_DirectionalRot, (float*)&scale, mat);
			ImGuizmo::Manipulate((float*)&g_Engine->camera.viewMatrix, (float*)&g_Engine->camera.projectionMatrix, ImGuizmo::ROTATE, ImGuizmo::LOCAL, (float*)&mat);
			ImGuizmo::DecomposeMatrixToComponents(mat, (float*)&translate, (float*)&g_Lighting_DirectionalRot, (float*)&scale);
			ImGuizmo::DrawDirectionArrow();
		}
	}
	ImGui::End();
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
	}
	ImGui::EndMainMenuBar();
}

void RenderMenus()
{
	RenderMainMenu();

	if(g_cameraWindowOpen) RenderCameraMenu(&g_cameraWindowOpen);
	if (g_lightingWindowOpen) RenderLightingWindow(&g_lightingWindowOpen);
}

} // namespace ImGui::Integration