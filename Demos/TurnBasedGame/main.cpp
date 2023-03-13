
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/RNG.hpp>
#include <TLib/Media/Camera2DDebug.hpp>
#include <TLib/Pointers.hpp>
#include <TLib/Containers/Colony.hpp>

#include <libtcod/path.hpp>
#include <libtcod/pathfinder.h>

using PFMap = TCODMap;
using PFPath = TCODPath;

struct Config
{
    // Gameplay settings
    Vector2f gridSize = { 32.f, 32.f };

    // Controls
    Action actPrimary   = { "Primary",   { ActionType::MOUSE,    Input::MOUSE_LEFT  } };
    Action actSecondary = { "Secondary", { ActionType::MOUSE,    Input::MOUSE_RIGHT } };
    Action actRestart   = { "Restart",   { ActionType::KEYBOARD, SDL_SCANCODE_R     } };
} config;

struct Team
{
    ColorRGBAf color = ColorRGBAf::magenta();
    String name;
};

struct Unit : SafeObj
{
    String   name;
    Vector2i pos;
};

struct Map
{
    operator PFMap*() { return &map; }

private:
    PFMap map{10, 10};
    Colony<Unit> units;

public:

    const auto& getUnits() const
    { return units; }

    Vector2i getSize() const
    { return { map.getWidth(), map.getHeight() }; }

    Unit* getUnitAtPos(const Vector2i& pos)
    {
        for (auto& unit : units)
        {
            if (unit.pos == pos)
            { return &unit; }
        }
        return nullptr;
    }

    bool addUnit(const Unit& unit, const Vector2i& pos)
    {
        if (getUnitAtPos(pos)) { return false; }
        units.insert(unit)->pos = pos;
        map.setProperties(pos.x, pos.y, true, false);
        return true;
    }

    bool computePath(const Vector2i& start, const Vector2i& end, PFPath& outPath)
    {
        if (start.x < 0 || start.x >= map.getWidth()  ||
            start.y < 0 || start.y >= map.getHeight())
        {
            //tlog::error("computePath: Starting position {} outside the bounds of the pathfinding map.", start.toString());
            return false;
        }

        if (end.x < 0 || end.x >= map.getWidth()  ||
            end.y < 0 || end.y >= map.getHeight())
        {
            //tlog::error("computePath: Ending position {} outside the bounds of the pathfinding map.", start.toString());
            return false;
        }

        bool tmpStartTrans = map.isTransparent(start.x, start.y);
        bool tmpStartWalk  = map.isWalkable(start.x, start.y);
        bool tmpEndTrans   = map.isTransparent(end.x, end.y);
        bool tmpEndWalk    = map.isWalkable(end.x, end.y);
        bool endBlocked    = !tmpEndWalk;

        map.setProperties(start.x, start.y, true, true);
        map.setProperties(end.x, end.y, true, true);
        bool success = outPath.compute(start.x, start.y, end.x, end.y);
        map.setProperties(start.x, start.y, tmpStartTrans, tmpStartWalk);
        map.setProperties(end.x, end.y, tmpEndTrans, tmpEndWalk);

        if (success)
        {
            if (!endBlocked) { return true; }

            // Remove the last tile the actor at the end is occupying
            outPath.reverse();
            outPath.walk(NULL, NULL, false);
            outPath.reverse();
            return true;
        }
        return false;
    }

    void resetPath(PFPath& outPath)
    { outPath = PFPath(&map); }

    Map()
    {
        map.clear(true, true);
    }
};

void drawMap(const Map& map)
{
    auto size = map.getSize();
    Renderer2D::drawGrid({ 0, 0 }, { size.x, size.y }, config.gridSize);
    for (auto& unit : map.getUnits())
    {
        Renderer2D::drawCircle(Vector2f(unit.pos) * config.gridSize + config.gridSize / 2, 12.f);
    }
}

void drawPath(const Vector2i& start, const PFPath& path)
{
    if (path.size() == 0) { return; }

    Vector2i prev = start;
    const Vector2f halfGridSize = config.gridSize / 2;

    for (size_t i = 0; i < path.size(); i++)
    {
        Vector2i next;
        path.get(i, &next.x, &next.y);

        Renderer2D::drawLine(Vector2f(prev) * config.gridSize + halfGridSize, Vector2f(next) * config.gridSize + halfGridSize);
        prev = next;
    }
}

Vector2i posToGridPos(Vector2f pos, Vector2f gridSize)
{
    // Integer division truncates towards 0
    // Floor manually so it doesn't break with negative values
    return Vector2i((pos / gridSize).floored());
}

Window   window;
FPSLimit fpslimit;
Timer    deltaTimer;

struct Game
{
    RNG     rng;
    SDFFont font;

    Map map;
    SafePtr<Unit> selectedUnit;
    SafePtr<Unit> hoveredUnit;
    PFPath hoveredPath{ map };

    Vector2i worldGridPos;
    Vector2i lastWorldGridPos = { -INT_MAX, -INT_MAX };

    void start()
    {
        if (!font.created())
        { RELASSERTMSGBOX(font.loadFromFile("assets/roboto.ttf"), "Asset error", "Failed to load 'assets/roboto.ttf'"); }

        map.addUnit(Unit(), { 0, 1 });
        map.addUnit(Unit(), { 0, 0 });
        map.addUnit(Unit(), { 5, 5 });
    }

    void updateHoveredPath()
    {

    }

    void loop(float delta)
    {
        //// Update
        if (window.hasInputFocus())
        {
            Camera2D view = Renderer2D::getView();
            debugCamera(view);
            Renderer2D::setView(view);

            Rectf    screenRect    = view.getBounds();
            Vector2f mouseWorldPos = view.localToWorldCoords(Input::mousePos);

            worldGridPos = posToGridPos(mouseWorldPos, config.gridSize);

            // IF MOUSE MOVED TOO NEW GRID
            if (worldGridPos != lastWorldGridPos)
            {
                tlog::info(worldGridPos.toString());

                hoveredUnit = map.getUnitAtPos(worldGridPos);

                if (Input::isActionJustPressed(config.actPrimary))
                {
                    if (!hoveredUnit.empty())
                    { selectedUnit = hoveredUnit; }
                }

                if (!selectedUnit.empty())
                { map.computePath(selectedUnit->pos, worldGridPos, hoveredPath); }
                else
                { map.resetPath(hoveredPath); }

                lastWorldGridPos = worldGridPos;
            }
        }

        //// Draw
        Renderer::clearColor();

        drawMap(map);

        if (!selectedUnit.empty() && !hoveredPath.isEmpty())
        { drawPath(selectedUnit->pos, hoveredPath); }

        if (!hoveredUnit.empty())
        { Renderer2D::drawRect({ Vector2f(hoveredUnit->pos) * config.gridSize, config.gridSize }, 1, ColorRGBAf::green()); }

        const Vector2f offset = { 20, 20 };
        Renderer2D::drawText(fmt::format("Grid: ({}, {})\nlmao", worldGridPos.x, worldGridPos.y), font, Renderer2D::getView().getBoundsPos() + offset);
        Renderer2D::drawRect({ 20, 20, 100, 30 });

        Renderer2D::render();
    }
};

int main()
{
    window.create();
    window.setTitle("Turn Based Game");
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