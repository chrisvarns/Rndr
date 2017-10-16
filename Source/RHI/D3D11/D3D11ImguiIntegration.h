#pragma once
#include "Imgui/ImguiIntegration.h"

namespace RHI {

class RHI;

namespace D3D11 {

class D3D11ImGuiIntegration : public ImguiIntegration
{
public:
    virtual bool Init(SDL_Window* window, RHI* rhi) override;
    virtual void Shutdown() override;
    virtual void NewFrame(SDL_Window* window) override;
    virtual bool ProcessEvent(SDL_Event* event) override;

    // Use if you want to reset your rendering device without losing ImGui state.
    virtual void InvalidateDeviceObjects() override;
    virtual bool CreateDeviceObjects() override;
};

}
}