//
//#include "Media/Media.hpp"
//#include <format>
//
//Window window;
//Renderer renderer;
//
//const char* fontPath = "Content/arial.ttf";
//
//int main()
//{
//    window.create("Window", 1280, 720);
//    renderer.create(window);
//    FPSLimit fpslimit(60);
//    bool running = true;
//
//    Font font = renderer.createFont(fontPath);
//
//    renderer.setClearColor(ColorRGBAi{ 45, 45, 45 });
//    while (running)
//    {
//        Event e;
//        while (SDL_PollEvent(&e))
//        {
//            if (e.type == SDL_QUIT) { running = false; }
//        }
//
//        renderer.clear();
//
//        int x, y;
//        SDL_GetMouseState(&x, &y);
//        renderer.drawText(font, { 100, 100 }, std::format("Mouse pos: ({}, {})", x, y) );
//
//        renderer.present();
//        fpslimit.wait();
//    }
//}