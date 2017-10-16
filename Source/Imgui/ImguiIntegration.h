#pragma once

namespace RHI {
class RHI;
}

struct SDL_Window;
union SDL_Event;

class ImguiIntegration {
public:
    virtual bool Init(SDL_Window* window, RHI::RHI* rhi) = 0;
    virtual void Shutdown() = 0;
    virtual void NewFrame(SDL_Window* window) = 0;
    virtual bool ProcessEvent(SDL_Event* event) = 0;

    // Use if you want to reset your rendering device without losing ImGui state.
    virtual void InvalidateDeviceObjects() = 0;
    virtual bool CreateDeviceObjects() = 0;
};