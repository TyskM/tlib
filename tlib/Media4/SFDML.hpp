#pragma once

#define SFML_STATIC
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#undef main

// Use SDL input handling with SFML drawing
SDL_Window* initWindow(sf::RenderWindow& win, const char* name, int width, int height)
{
    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_Window* sdlwindow = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_RESIZABLE);
    SDL_SysWMinfo systemInfo; 
    SDL_VERSION(&systemInfo.version);
    SDL_GetWindowWMInfo(sdlwindow, &systemInfo);
    HWND handle = systemInfo.info.win.window;

    win.create(handle);
    return sdlwindow;
}

// int main()
// {
//     sf::RenderWindow window;
//     initWindow(window);
// 
//     bool running = true;
//     while (running)
//     {
// 
//         SDL_Event e;
//         while (SDL_PollEvent(&e))
//         {
//             if (e.window.event == SDL_WINDOWEVENT_CLOSE) running = false;
//         }
// 
//         window.clear({ 200, 20, 70 });
// 
//         window.display();
//     }
//     return EXIT_SUCCESS;
// }