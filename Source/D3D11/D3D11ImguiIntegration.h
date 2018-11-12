#pragma once

class D3D11RHI;
struct SDL_Window;
union SDL_Event;

class D3D11ImGuiIntegration
{
public:
    bool Init(SDL_Window* window, D3D11RHI* rhi);
    void Shutdown();
    void NewFrame(SDL_Window* window);
    bool ProcessEvent(SDL_Event* event);

    // Use if you want to reset your rendering device without losing ImGui state.
    void InvalidateDeviceObjects();
    bool CreateDeviceObjects();
};