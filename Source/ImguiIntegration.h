#pragma once
struct ID3D11Device;
struct ID3D11DeviceContext;
union SDL_Event;
struct SDL_Window;

namespace ImGui::Integration
{
	bool        Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* device_context);
	void        Shutdown();
	void        NewFrame(SDL_Window* window);
	bool		ProcessEvent(SDL_Event* event);

	// Use if you want to reset your rendering device without losing ImGui state.
	void        InvalidateDeviceObjects();
	bool        CreateDeviceObjects();
}