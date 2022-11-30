#pragma once

#define SFML_STATIC
#define NOMINMAX
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <NonAssignable.hpp>
#undef main

using sf::Texture;
using sf::Sprite;
using sf::Shape;
using sf::CircleShape;
using sf::RectangleShape;

struct Window
{
    std::unique_ptr<sf::RenderWindow> sfmlwindow;
    SDL_Window* sdlwindow;
};

// Use SDL input handling with SFML drawing
Window createWindow(const char* name, int width, int height)
{
    Window window;

    SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
    SDL_Window* sdlwindow = SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_RESIZABLE);
    SDL_SysWMinfo systemInfo; 
    SDL_VERSION(&systemInfo.version);
    SDL_GetWindowWMInfo(sdlwindow, &systemInfo);
    HWND handle = systemInfo.info.win.window;

    window.sfmlwindow = std::make_unique<sf::RenderWindow>();
    window.sfmlwindow->create(handle);
    return window;
}

//sf::RenderWindow window;
//SDL_Window* sdlWin = initWindow(window, "Window Title", 600, 600);
//MyGui::init(sdlWin);
//
//bool running = true;
//while (running)
//{
//    SDL_Event e;
//    while (SDL_PollEvent(&e))
//    {
//        if (e.window.event == SDL_WINDOWEVENT_CLOSE) running = false;
//        MyGui::input(e);
//    }
//
//    MyGui::newFrame();
//    window.clear({ 200, 20, 70 });
//
//    MyGui::render(window);
//    window.display();
//}
//return EXIT_SUCCESS;