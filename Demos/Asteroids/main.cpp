
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/RNG.hpp>

// TODO:
// Audio ie. Hitsounds, shoot sounds, death sound, high score sound, field cleared sound
// Score saving, high scores
// Floating score text on hit
// VFX for death, hit
// Background gfx
// Score bonus for accuracy && clear speed

struct Config
{
    // Gameplay settings
    float playerRotSpeed         = 5.f;
    float playerMoveSpeed        = 2.f;
    float playerHitboxSize       = 10.f;
    float projectileSize         = 4.f;
    float projectileSpeed        = 360.f;
    float projectileLifetimeSecs = 1.4f;
    float asteroidMaxSize        = 18.f;
    float asteroidMinSize        = asteroidMaxSize / 3.f + 1.f;

    // Controls
    Action actForward  = { "Forward",  { ActionType::KEYBOARD, SDL_SCANCODE_W     } };
    Action actBackward = { "Backward", { ActionType::KEYBOARD, SDL_SCANCODE_S     } };
    Action actLeft     = { "Left",     { ActionType::KEYBOARD, SDL_SCANCODE_A     } };
    Action actRight    = { "Right",    { ActionType::KEYBOARD, SDL_SCANCODE_D     } };
    Action actPrimary  = { "Primary", {{ ActionType::MOUSE,    Input::MOUSE_LEFT  },
                                       { ActionType::KEYBOARD, SDL_SCANCODE_SPACE }}};
    Action actRestart  = { "Restart",  { ActionType::KEYBOARD, SDL_SCANCODE_R     } };
    Action actDbgAsteroid =
    { "Debug Spawn Asteroid", { ActionType::KEYBOARD, SDL_SCANCODE_E } };
} config;

struct Projectile
{
    Timer    timeAlive;
    Vector2f pos;
    Vector2f vel;
    bool     freed = false;
};

struct Asteroid
{
    Vector2f pos;
    Vector2f vel;
    int      segmentCount = 6;
    float    radius = 16.f;
    bool     freed = false;
};

struct Player
{
    Vector2f pos;
    Vector2f vel;
    Vector2f size = (Vector2f{ 32.f, 32.f });
    float    rot  = 0.f;
};



bool circlesIntersect(const Vector2f& c1pos, float c1rad, const Vector2f& c2pos, float c2rad)
{
    return c1pos.distanceTo(c2pos) < c2rad + c1rad;
}

Vector2f getWrappedCoord(const Vector2f& in, const Rectf& bounds)
{
    Vector2f out = in;
    if (in.x < bounds.x)      { out.x = in.x + bounds.width;  }
    if (in.x > bounds.width)  { out.x = in.x - bounds.width;  }
    if (in.y < bounds.y)      { out.y = in.y + bounds.height; }
    if (in.y > bounds.height) { out.y = in.y - bounds.height; }
    return out;
}

Window     window;
FPSLimit   fpslimit;
Timer      deltaTimer;

struct Game
{
    Vector<Asteroid>   asteroids;
    Vector<Projectile> projectiles;
    Player  player;
    RNG     rng;

    SDFFont font;
    Texture shipTex;
    Texture plumeTex;

    bool   gameOver = false;
    double score = 0.f;

    void start()
    {
        if (!shipTex.created())
        { RELASSERTMSGBOX(shipTex.loadFromFile("assets/ship.png"), "Asset error", "Failed to load 'assets/ship.png'"); }
        shipTex.setFilter(TextureFiltering::Nearest);

        if (!plumeTex.created())
        { RELASSERTMSGBOX(plumeTex.loadFromFile("assets/plume.png"), "Asset error", "Failed to load 'assets/plume.png'"); }
        plumeTex.setFilter(TextureFiltering::Nearest);

        if (!font.created())
        { RELASSERTMSGBOX(font.loadFromFile("assets/roboto.ttf"), "Asset error", "Failed to load 'assets/roboto.ttf'"); }

        asteroids.clear();
        projectiles.clear();
        gameOver   = false;
        score      = 0.f;
        player     = Player();
        player.pos = Vector2f(window.getFramebufferSize()) / 2.f;
    }

    void loop(float delta)
    {
        Camera2D cam           = Renderer2D::getView();
        Rectf    screenRect    = cam.getBounds();
        Vector2f mouseWorldPos = cam.localToWorldCoords(Input::mousePos);
        float fb = 0.f;

        // Update
        if (!gameOver)
        {
            float lr   = Input::isActionPressed(config.actRight)    - Input::isActionPressed(config.actLeft);
            fb         = Input::isActionPressed(config.actBackward) - Input::isActionPressed(config.actForward);
            bool shoot = Input::isActionJustPressed(config.actPrimary);

            // Debug asteroid
            if (Input::isActionJustPressed(config.actDbgAsteroid))
            {
                Asteroid a;
                a.segmentCount = rng.randRangeInt(4, 16);

                a.pos = mouseWorldPos;
                a.vel = Vector2f(Input::mouseDelta);
                asteroids.push_back(a);
            }

            // Player movement
            Vector2f playerLookVector = Vector2f(0, 1).rotated(player.rot);
            if (lr) { player.rot += lr * config.playerRotSpeed * delta; }
            if (fb) { player.vel += (playerLookVector * fb) * config.playerMoveSpeed; }
            player.pos += player.vel * delta;

            // Player Wrapping
            player.pos.x = fmod(player.pos.x, screenRect.width);
            player.pos.y = fmod(player.pos.y, screenRect.height);
            player.pos   = getWrappedCoord(player.pos, screenRect);

            // Shooting input
            if (shoot)
            {
                Projectile p;
                p.pos = player.pos;
                p.vel = (-playerLookVector * config.projectileSpeed);
                projectiles.push_back(p);
            }
        }

        if (Input::isActionJustPressed(config.actRestart))
        { start(); return; }

        // Asteroids movement and player collision
        for (auto& a : asteroids)
        {
            a.pos += a.vel * delta;
            a.pos = getWrappedCoord(a.pos, screenRect);
            if (circlesIntersect(player.pos, config.playerHitboxSize, a.pos, a.radius))
            { gameOver = true; }
        }

        // Projectile movement and collision
        for (auto& p : projectiles)
        {
            p.pos += p.vel * delta;
            p.pos = getWrappedCoord(p.pos, screenRect);

            Vector<Asteroid> newAsteroids;
            for (auto& a : asteroids)
            {
                // On intersect with an asteroid
                if (circlesIntersect(p.pos, config.projectileSize, a.pos, a.radius))
                {
                    p.freed = true;
                    a.freed = true;

                    score += 100.0 / a.radius;

                    if (a.radius < config.asteroidMinSize)
                    { break; }

                    // Split the rocks!
                    Asteroid newa1 = a;
                    newa1.freed    = false;
                    newa1.radius   = a.radius * 0.66666f;
                    Asteroid newa2 = newa1;

                    const float turnAmount = 0.133333f;
                    newa1.vel.rotate((math::pi * 2.f) *  turnAmount);
                    newa2.vel.rotate((math::pi * 2.f) * -turnAmount);

                    newAsteroids.push_back(newa1);
                    newAsteroids.push_back(newa2);

                    break;
                }
            }

            if (p.timeAlive.getElapsedTime().asSeconds() > config.projectileLifetimeSecs)
            { p.freed = true; }

            if (newAsteroids.size() > 0)
            {
                asteroids.insert(asteroids.end(), newAsteroids.begin(), newAsteroids.end());
                newAsteroids.clear();
            }
        }

        // Erase freed projectiles and asteroids
        eastl::erase_if(asteroids,   [](const auto& v) { return v.freed; });
        eastl::erase_if(projectiles, [](const auto& v) { return v.freed; });

        // Spawn new asteroids if there's none left
        if (asteroids.size() == 0)
        {
            const float distFromPlayer = 250.f;
            Asteroid a;
            a.radius = config.asteroidMaxSize;
            a.segmentCount = rng.randRangeInt(4, 16);
            a.pos = player.pos;
            a.pos.x -= distFromPlayer;
            a.vel = Vector2f(rng.randRangeReal(1.f, 100.f), rng.randRangeReal(1.f, 100.f));
            asteroids.push_back(a);

            a.segmentCount = rng.randRangeInt(4, 16);
            a.pos = player.pos;
            a.pos.x += distFromPlayer;
            a.vel = Vector2f(rng.randRangeReal(1.f, 100.f), rng.randRangeReal(1.f, 100.f));
            asteroids.push_back(a);
        }

        // Draw
        Renderer::clearColor();

        for (auto& a : asteroids)
        { Renderer2D::drawCircle(a.pos, a.radius, 0, ColorRGBAf::white(), false, a.segmentCount); }

        for (auto& p : projectiles)
        { Renderer2D::drawCircle(p.pos, config.projectileSize, 0, ColorRGBAf::yellow(), true, 5); }

        if (!gameOver)
        {
            const Vector2f playerCenteredPos = player.pos - player.size/2.f;;

            Renderer2D::drawTexture(shipTex,         { playerCenteredPos, player.size }, 0, ColorRGBAf::white(), player.rot);


            if (fb != 0.0f)
            { Renderer2D::drawTexture(plumeTex, { playerCenteredPos, player.size }, 0, ColorRGBAf::white(), player.rot); }
            
            Renderer2D::drawText(fmt::format("Score: {:.0f}", score), font, { 20, 20 });
        }
        else
        {
            Vector2f screenCenter = Vector2f(screenRect.width, screenRect.height) / 2;
            String text1 = "Game Over";
            String text2 = fmt::format("Final Score: {:.0f}", score);
            Vector2f text1Size = font.calcTextSize(text1);
            Vector2f text2Size = font.calcTextSize(text2);
            float padding = 12.f;

            Renderer2D::drawText(text1, font, { screenCenter.x - text1Size.x/2, screenCenter.y });
            Renderer2D::drawText(text2, font, { screenCenter.x - text2Size.x/2, screenCenter.y + text1Size.y + padding });
        }

        Renderer2D::render();
    }
};

int main()
{
    window.create();
    window.setTitle("Asteroids");
    Renderer::create();
    Renderer2D::create();

    deltaTimer .restart();
    fpslimit   .setFPSLimit(144);
    fpslimit   .setEnabled(true);

    Game game;
    game.start();

    bool running = true;
    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = Renderer2D::getView();
                view.setBoundsSize(Vector2f(e.window.data1, e.window.data2));
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        Input::update();

        game.loop(delta);

        window.swap();

        fpslimit.wait();
    }

    return 0;
}