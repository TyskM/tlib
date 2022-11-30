
#pragma once

#include "Window.hpp"
#include "Renderer.hpp"
#include "FPSLimit.hpp"
#include "Event.hpp"
#include "Input.hpp"
#include "Font.hpp"

// Simple game loop example:
//
//  Window window;
//  Renderer renderer;
//  
//  int main()
//  {
//      window.create("Window", 1280, 720);
//      renderer.create(window);
//      FPSLimit fpslimit(60);
//      bool running = true;
//  
//      while (running)
//      {
//          Event e;
//          while (SDL_PollEvent(&e))
//          {
//              if (e.type == SDL_QUIT) { running = false; }
//          }
//  
//          renderer.clear();
//  
//  
//          renderer.present();
//          fpslimit.wait();
//      }
//  }