#pragma once

#include "UniquePtr.h"
#include "sdl/SDL.h"

class Window
{
public:
    Window(uint32_t width, uint32_t height)
        : width(width)
        , height(height)
    {}

    uint32_t		width;
    uint32_t		height;
    UniquePtr<SDL_Window> sdlWindow = UniquePtr<SDL_Window>([](SDL_Window* window) { SDL_DestroyWindow(window); });
    SDL_SysWMinfo m_SdlWindowWMInfo;
};
