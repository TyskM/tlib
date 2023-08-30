
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/RNG.hpp>
#include <TLib/Media/ImGui.hpp>
#include <TLib/Containers/Set.hpp>

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

    //// Controls

    // Don't let users change this
    Action actMenu     = { "Menu",     { ActionType::KEYBOARD, SDL_SCANCODE_ESCAPE } };

    Action actForward  = { "Forward",  {  ActionType::KEYBOARD, SDL_SCANCODE_W      } };
    Action actBackward = { "Backward", {  ActionType::KEYBOARD, SDL_SCANCODE_S      } };
    Action actLeft     = { "Left",     {  ActionType::KEYBOARD, SDL_SCANCODE_A      } };
    Action actRight    = { "Right",    {  ActionType::KEYBOARD, SDL_SCANCODE_D      } };
    Action actPrimary  = { "Shoot",    {{ ActionType::MOUSE,    Input::MOUSE_LEFT   },
                                       {  ActionType::KEYBOARD, SDL_SCANCODE_SPACE  }}};
    Action actRestart  = { "Restart",  {  ActionType::KEYBOARD, SDL_SCANCODE_R      } };
} config;

struct Projectile
{
    float    timeAlive = 0.f;
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
MyGui      imgui;
bool       running = true;

struct Game
{
    Vector<Asteroid>   asteroids;
    Vector<Projectile> projectiles;
    Player  player;
    RNG     rng;

    SDFFont font;
    Texture shipTex;
    Texture plumeTex;

    bool   paused = false;
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

        ////////// Update
        if (!paused)
        {
            if (!gameOver)
            {
                float lr   = Input::isActionPressed(config.actRight)    - Input::isActionPressed(config.actLeft);
                fb         = Input::isActionPressed(config.actBackward) - Input::isActionPressed(config.actForward);
                bool shoot = Input::isActionJustPressed(config.actPrimary);

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

                p.timeAlive += delta;
                if (p.timeAlive > config.projectileLifetimeSecs)
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
        }

        ////////// Draw
        Renderer::clearColor();

        for (auto& a : asteroids)
        { Renderer2D::drawCircle(a.pos, a.radius, 0, ColorRGBAf::white(), false, a.segmentCount); }

        for (auto& p : projectiles)
        { Renderer2D::drawCircle(p.pos, config.projectileSize, 0, ColorRGBAf::yellow(), true, 5); }

        if (!gameOver)
        {
            const Vector2f playerCenteredPos = player.pos - player.size/2.f;;

            Renderer2D::drawTexture(shipTex, { playerCenteredPos, player.size }, 0, ColorRGBAf::white(), player.rot);

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
} game;

struct Menu
{
private:
    enum class State
    {
        Main,
        Input
    };

    Texture backTex;

    bool _open = false;
    Vector<State> stateStack = {State::Main};
    Vector<Action*> actions;
    
    Input::ActionControl* keyToAssign = nullptr;
    Set<Input::ActionControl*> keysToRemove;

public:
    void setup()
    {
        backTex.loadFromFile("assets/left.png");

        actions.push_back(&config.actForward);
        actions.push_back(&config.actBackward);
        actions.push_back(&config.actLeft);
        actions.push_back(&config.actRight);
        actions.push_back(&config.actPrimary);
        actions.push_back(&config.actRestart);
    }

    float backButtonSize = 16.f;
    int maxActions = 3;
    Vector2f mainWindowSize  = { 130, 180 };
    Vector2f inputWindowSize = { 300, 350 };

    bool isOpen() const { return _open == true; }

    void open()
    {
        _open = true;
        game.paused = true;
    }

    void close()
    {
        _open = false;
        stateStack = {State::Main};
        keyToAssign = nullptr;
        keysToRemove.clear();
        game.paused = false;
    }

    void toggle()
    {
        if (isOpen()) { close(); }
        else { open(); }
    }

    void render()
    {
        if (!_open) { close(); return; }

        if (stateStack.back() != State::Main)   { ImGui::SetNextWindowSize({ inputWindowSize.x, inputWindowSize.y }); }
        else                                    { ImGui::SetNextWindowSize({ mainWindowSize.x,  mainWindowSize.y  }); }
        ImGui::Begin("Menu", &_open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        // Back button
        if (stateStack.size() > 1)
        {
            if (ImGui::ImageButton("Back", (ImTextureID)backTex.handle(), {backButtonSize, backButtonSize}))
            { stateStack.pop_back(); }
        } else
        { ImGui::Dummy({backButtonSize, backButtonSize}); }

        switch (stateStack.back())
        {
        case State::Main:  mainMenu();  break;
        case State::Input: inputMenu(); break;
        default: break;
        }

        ImGui::End();
    }

private:
    void mainMenu()
    {
        if (ImGui::Button("Restart", {-1, 0}))
        { game.start(); close(); }
        if (ImGui::Button("Key Mapping", {-1, 0}))
        { stateStack.push_back(State::Input); }
        if (ImGui::Button("Quit", {-1, 0}))
        { running = false; }
    }

    void inputMenu()
    {
        auto& io = ImGui::GetIO();

        const float buttonWidth = 80.f;

        if (keyToAssign)
        {
            if (Input::getLastInput().valid())
            {
                *keyToAssign = Input::getLastInput();
                keyToAssign = nullptr;
            }
        }

        int i = 0;
        const int columns = maxActions + 1;
        if (ImGui::BeginTable("table", columns, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingFixedSame))
        {
            for (Input::Action* act : actions)
            {
                ImGui::TableNextColumn();
                ImGui::Text(act->name.c_str());
                ImGui::TableNextColumn();

                // Draw/edit current keybinds
                for (Input::ActionControl& c : act->controls)
                {
                    String name;
                    if (&c == keyToAssign) name = "...";
                    else                   name = Input::controlToStringShort(c);

                    if (ImGui::Button((name + "##" + std::to_string(i)).c_str(), {-1, 0}))
                    {
                        keyToAssign = &c;
                        Input::clearLastInput();
                    }

                    // Remove input
                    if (ImGui::IsItemHovered() && io.MouseDown[1])
                    {
                        keysToRemove.insert(&c);
                    }

                    ImGui::TableNextColumn();
                    ++i;
                }

                // Add keybind
                if (act->controls.size() < maxActions)
                {
                    if (ImGui::Button( String("+##" + std::to_string(i)).c_str(), {-1, 0} ))
                    {
                        act->controls.push_back(Input::ActionControl(Input::ActionType::KEYBOARD, SDL_SCANCODE_AGAIN));
                        keyToAssign = &act->controls.back();
                        Input::clearLastInput();
                    }
                    ++i;
                }
                ImGui::TableNextRow();
            }

            ImGui::EndTable();
        }

        // Remove keys queued for delete
        if (keysToRemove.empty()) { return; }
        for (auto& act : actions)
        {
            eastl::erase_if(act->controls, [this](Input::ActionControl& v)
                {
                    auto it = keysToRemove.find(&v);
                    if (it != keysToRemove.end())
                    { return true; }
                    return false;
                });
        }
        keysToRemove.clear();
    }

} menu;

int main()
{
    bool showMetricsWindow = false;

    window.create();
    window.setTitle("Asteroids");
    Renderer::create();
    Renderer2D::create();
    imgui.create(window);
    auto& io = ImGui::GetIO();
    const ImWchar range[3] = {0x20, 0xFFFF, 0};
    io.Fonts->AddFontFromFileTTF("assets/roboto.ttf", 16, 0, range);

    menu.setup();

    deltaTimer .restart();
    fpslimit   .setFPSLimit(144);
    fpslimit   .setEnabled(true);

    game.start();

    while (running)
    {
        float delta = deltaTimer.restart().asSeconds();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            Input::input(e);
            imgui.input(e);

            if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
            {
                auto view = Renderer2D::getView();
                view.setBoundsSize(Vector2f(e.window.data1, e.window.data2));
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse))    { Input::updateMouse(); }

        if (Input::isActionJustPressed(config.actMenu))
        { menu.toggle(); }

        if (Input::isKeyJustPressed(SDL_SCANCODE_F1))
        { showMetricsWindow = !showMetricsWindow; }

        imgui.newFrame();
        if (showMetricsWindow) { ImGui::ShowMetricsWindow(&showMetricsWindow); }
        game.loop(delta);
        menu.render();
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}