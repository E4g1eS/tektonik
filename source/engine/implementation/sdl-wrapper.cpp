#include "sdl-wrapper.hpp"

#include "std.hpp"

bool RunDemoSdlApp()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cout << SDL_GetError() << std::endl;
        return false;
    }

    SDL_Window* window = SDL_CreateWindow("SDL3 Minimal Example", 800, 600, SDL_WINDOW_VULKAN);

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }
        }
        // Optionally clear, render, etc.
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return true;
}
