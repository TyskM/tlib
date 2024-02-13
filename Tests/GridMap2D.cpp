
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Misc.hpp>

#include <TLib/Containers/GridMap2D.hpp>

Font font;

bool drawNodeTree = true;
bool drawNodeCost = true;

constexpr float gridSize     = 32.f;
constexpr float halfGridSize = gridSize / 2.f;

GridMap2D        map;
Vector2i         start;
Vector2i         goal;
Vector<Vector2i> path;
Vector<Vector2f> worldSpacePath;

UnorderedMap<Vector2i, Vector2i> cameFrom;
UnorderedMap<Vector2i, float>    costSoFar;

Vector2f gridToWorldSpace(const Vector2i& gridPos)
{
    return Vector2f(gridPos * gridSize + halfGridSize);
}

void drawCircleAtGrid(const Vector2i& gridPos, float rad,
                      bool filled = false, ColorRGBAf color = ColorRGBAf::white())
{
    Renderer2D::drawCircle(gridToWorldSpace(gridPos), halfGridSize, filled, color);
}

void drawTriAtGrid(const Vector2i& gridPos, float rot, bool filled = false, ColorRGBAf color = ColorRGBAf::white())
{
    Renderer2D::drawTriangle(gridToWorldSpace(gridPos), Vector2f(halfGridSize), rot, filled, color);
}

void drawMap()
{
    Renderer2D::drawGrid({ 0,0 }, map.getSize(), gridSize);

    for   (int x = 0; x <  map.width(); x++)
    { for (int y = 0; y < map.height(); y++)
    {
        Rectf rect(Vector2f(x, y) * gridSize, gridSize);
        if (!map.getGridAt(x, y).passable)
        { Renderer2D::drawRect(rect, 0.f, true, ColorRGBAf::white()); }
    }}

    if (drawNodeTree)
    {
        for (auto& [k, v] : cameFrom)
        {
            auto& next = v;

            const ColorRGBAf triColor = ColorRGBAf::yellow().setA(0.4f);

            if      (next.x == k.x + 1 && next.y == k.y - 1) { drawTriAtGrid(k, glm::radians(45.f), false, triColor); }
            else if (next.x == k.x + 1 && next.y == k.y + 1) { drawTriAtGrid(k, glm::radians(45.f * 3), false, triColor); }
            else if (next.x == k.x - 1 && next.y == k.y + 1) { drawTriAtGrid(k, glm::radians(45.f * 5), false, triColor); }
            else if (next.x == k.x - 1 && next.y == k.y - 1) { drawTriAtGrid(k, glm::radians(45.f * 7), false, triColor); }

            else if (next.x == k.x + 1) { drawTriAtGrid(k, glm::radians(90.f), false, triColor); }
            else if (next.x == k.x - 1) { drawTriAtGrid(k, glm::radians(90.f * 3), false, triColor); }
            else if (next.y == k.y + 1) { drawTriAtGrid(k, glm::radians(90.f * 2), false, triColor); }
            else if (next.y == k.y - 1) { drawTriAtGrid(k, glm::radians(0.f), false, triColor); }
        }
    }

    if (worldSpacePath.size() > 0)
    { Renderer2D::drawLines(worldSpacePath, ColorRGBAf::blue(), GLDrawMode::LineStrip); }

    drawCircleAtGrid(start, halfGridSize, true, ColorRGBAf::green());
    drawCircleAtGrid(goal, halfGridSize, true, ColorRGBAf::red());

    if (drawNodeCost)
    {
        for (auto& [k, v] : costSoFar)
        {
            Renderer2D::drawText(floatToStr(v, 1), font, gridToWorldSpace(k) + Vector2f{-14.f, 9.f}, ColorRGBAf::azure(), 0.5f);
        }
    }
}

void refreshPath()
{
    path = map.computePath(start, goal, &cameFrom, &costSoFar);
    tlog::info("Calculated path with {} points.", path.size());

    worldSpacePath.clear();
    worldSpacePath.reserve(path.size());
    for (auto& p : path)
    {
        worldSpacePath.push_back(Vector2f(p) * gridSize + halfGridSize);
    }
}

void setStart(const Vector2i& pos)
{ start = pos; refreshPath(); }

void setGoal(const Vector2i& pos)
{ goal = pos; refreshPath(); }

void setPassable(const Vector2i& pos, bool value)
{
    auto& g = map.getGridAt(pos);
    if (g.passable == value) { return; }
    g.passable = value;
    refreshPath();
}

void gridMapInput()
{
    Vector2f mousePos     = Renderer2D::getMouseWorldPos();
    Vector2i mouseGridPos = posToGridPos(mousePos, gridSize);

    if (!map.inBounds(mouseGridPos))
    { return; }

    if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT))
    {
        if      (Input::isMouseJustPressed(Input::MOUSE_LEFT))
        { setStart(mouseGridPos); }
        else if (Input::isMouseJustPressed(Input::MOUSE_RIGHT))
        { setGoal(mouseGridPos); }
    }
    else
    {
        if      (Input::isMousePressed(Input::MOUSE_LEFT))
        { setPassable(mouseGridPos, false); }
        else if (Input::isMousePressed(Input::MOUSE_RIGHT))
        { setPassable(mouseGridPos, true); }
    }
}

int main()
{
    Window     window;
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;
    WindowCreateParams p;
    p.size = { 1280, 720 };
    window.create(p);
    window.setTitle("GridMap2D");
    Renderer::create();
    Renderer2D::create();
    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    font.loadFromFile("assets/roboto.ttf", 24);
    map.includeStart = true;
    map.setSize(20, 10);
    setGoal(map.getSize() - 1);

    bool running = true;
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
                view.size = Vector2f(e.window.data1, e.window.data2);
                Renderer2D::setView(view);
            }

            if (e.type == SDL_QUIT) { running = false; }
        }
        auto& io = ImGui::GetIO();
        if (!(io.WantCaptureKeyboard)) { Input::updateKeyboard(); }
        if (!(io.WantCaptureMouse))    { Input::updateMouse(); }

        imgui.newFrame();

        auto view = Renderer2D::getView();
        debugCamera(view);
        Renderer2D::setView(view);

        gridMapInput();

        beginDiagWidgetExt();
        static int mapSizeInput[2] = { map.getSize().x, map.getSize().y };
        if (ImGui::InputInt2("Map Size", mapSizeInput,
            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
        {
            map.setSize(Vector2i(mapSizeInput[0], mapSizeInput[1]));
        }
        ImGui::Checkbox("Draw Node Tree", &drawNodeTree);
        ImGui::Checkbox("Draw Node Cost (Perf Heavy)", &drawNodeCost);
        if (ImGui::SliderFloat("Diagonal Cost", &map.diagonalCost, 0.0f, 3.f))
        { refreshPath(); }
        ImGui::End();

        Renderer::clearColor();
        drawMap();
        Renderer2D::render();

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}