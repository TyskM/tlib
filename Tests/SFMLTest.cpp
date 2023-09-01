
#include <SFML/Graphics.hpp>
#include <TLib/DataStructures.hpp>

// For comparison to SimpleSpriteTest
struct SFMLTest
{
    sf::RenderWindow win;
    sf::Clock deltaClock;

    sf::Texture tex;
    sf::Sprite  sprite;

    // FPS stuff
    sf::Clock timer;
    int fpscounter = 0;

    const int spriteCount = 20000;

    void run()
    {
        win.create(sf::VideoMode(1280, 720), "SFML Test");
        tex.loadFromFile("assets/ship.png");
        sprite.setTexture(tex);

        while (win.isOpen())
        {
            sf::Event event;
            while (win.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                { win.close(); }
            }

            win.clear({ 25, 25, 25, 255 });
            
            float delta = deltaClock.restart().asSeconds();
            static float time = 0.f;
            time += delta;
            int count = spriteCount;
            int sr = std::ceil(sqrt(count));

            for (int x = 0; x < sr; x++)
            {
                for (int y = 0; y < sr; y++)
                {
                    const float rot = sin(time) * x + y;
                    const sf::Color color =
                    {
                        floatToUint8(fmodf(sin(time), 1.f) * (y%16)),
                        floatToUint8(fmodf(cos(time / 2.f) * (x%12), 1.f)),
                        floatToUint8(fmodf((time)+x+y, 1.f)),
                        255
                    };

                    sprite.setColor(color);
                    sprite.setPosition(sf::Vector2f(x, y) * 4.f);
                    sprite.setRotation(rot);
                    win.draw(sprite);

                    --count;
                    if (count == 0) break;
                }
                if (count == 0) break;
            }
            
            
            
            win.display();

            if (timer.getElapsedTime().asSeconds() > 1.f)
            {
                win.setTitle(std::string("SFML FPS: ") + std::to_string(fpscounter));
                fpscounter = 0;
                timer.restart();
            }
            ++fpscounter;
        }
    }
};

int main()
{
    SFMLTest game;
    game.run();
    return 0;
}