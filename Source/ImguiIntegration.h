#pragma once
struct ID3D11Device;
struct ID3D11DeviceContext;
union SDL_Event;
struct SDL_Window;

bool        ImguiIntegration_Init(SDL_Window* window, ID3D11Device* device, ID3D11DeviceContext* device_context);
void        ImguiIntegration_Shutdown();
void        ImguiIntegration_NewFrame(SDL_Window* window);
bool		ImguiIntegration_ProcessEvent(SDL_Event* event);

// Use if you want to reset your rendering device without losing ImGui state.
void        ImguiIntegration_InvalidateDeviceObjects();
bool        ImguiIntegration_CreateDeviceObjects();