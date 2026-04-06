
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Misc.hpp>

#include <TLib/Containers/AStar2D.hpp>

/*
    An example of a custom grid type for use with the GridMap2D template.
    This is so you dont have to store all your per grid data in a separate array.
    (You still can if you want)
    Your type must inherit GridMap2DGrid.

    struct MyAwesomeGridStruct : public AStar2DGrid
    {
        // Usually you would store an id for a grid def here.
        // Or you can store all your data in each grid, follow your dreams.
        float awesomeness = FLT_MAX;
        int id;
    }

    AStar2D<MyAwesomeGridStruct> myAwesomeGridMap;
*/

struct Grid : public AStar2DGrid
{
    char icon = '.';
};

Font uiFont;
Font iconFont;

bool drawNodeTree  = false;
bool drawNodeCost  = false;
bool drawRayCast   = false;
bool drawGridIcons = true;

constexpr float gridSize     = 32.f;
constexpr float halfGridSize = gridSize / 2.f;

AStar2D<Grid>        map{20, 10};
Vector2i             start;
Vector2i             goal;
float                diagonalCost   = 1.001f;
bool                 allowDiagonals = true;
AStar2DPath          path;
Vector<Vector2f>     worldSpacePath;
AStar2DRaycastResult raycast;
Vector<Vector2i>     raycastPath;

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
    Renderer2D::drawGrid({ 0,0 }, map.size(), gridSize);

    for   (int x = 0; x <  map.width(); x++)
    { for (int y = 0; y < map.height(); y++)
    {
        Rectf rect(Vector2f(x, y) * gridSize, gridSize);
        auto& grid = map.at(x, y);
        if (!grid.passable)
        {
            Renderer2D::drawRect(rect, 0.f, true, ColorRGBAf::white());
        }
    }}

    if (drawGridIcons)
    {
        for   (int x = 0; x < map.width();  x++)
        { for (int y = 0; y < map.height(); y++)
        {
            Vector2f pos = Vector2f(x, y) * gridSize + gridSize/2.f;
            Rectf rect(Vector2f(x, y) * gridSize, gridSize);
            auto& grid = map.at(x, y);
            if (!grid.passable)
            { Renderer2D::drawChar(grid.icon, iconFont, pos, 0.f, ColorRGBAf::black()); }
            else
            { Renderer2D::drawChar(grid.icon, iconFont, pos, 0.f, ColorRGBAf::white()); }
        }}
    }

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

    if (drawRayCast)
    {
        ColorRGBAf rayColor = ColorRGBAf::darkGreen();
        if (raycast.hit) { rayColor = ColorRGBAf::darkRed(); }
        Renderer2D::drawLine(gridToWorldSpace(start), gridToWorldSpace(raycast.pos), rayColor);
        for (auto& grid : raycastPath)
        {
            Rectf rect(Vector2f(grid) * gridSize, gridSize);
            Renderer2D::drawRect(rect, 0.f, true, rayColor.setA(0.3f));
        }
    }

    if (drawNodeCost)
    {
        for (auto& [k, v] : costSoFar)
        {
            Renderer2D::drawText(floatToStr(v, 1), iconFont, gridToWorldSpace(k) + Vector2f{-14.f, 9.f}, ColorRGBAf::azure(), 0.5f);
        }
    }
}

void refreshPath()
{
    map.cameFromPtr  = &cameFrom;
    map.costSoFarPtr = &costSoFar;
    path = map.computePath(start, goal, allowDiagonals ? diagonalCost : FLT_MAX);
    tlog::info("Calculated path with {} points.", path.nodes.size());

    worldSpacePath.clear();
    worldSpacePath.reserve(path.nodes.size());
    for (auto& p : path.nodes)
    { worldSpacePath.push_back(Vector2f(p) * gridSize + halfGridSize); }

    raycast = map.raycast(start, goal, &raycastPath);
}

void setStart(const Vector2i& pos)
{ start = pos; refreshPath(); }

void setGoal(const Vector2i& pos)
{ goal = pos; refreshPath(); }

void setPassable(const Vector2i& pos, bool value)
{
    auto& g = map.at(pos);
    if (g.passable == value) { return; }
    g.passable = value;
    if (value) { g.icon = '.'; }
    else       { g.icon = '#'; }
    refreshPath();
}

void gridMapInput()
{
    const auto inputMMB    = Input::ActionControl(Input::ActionType::MOUSE,    Input::MOUSE_MIDDLE);
    const auto inputMWUp   = Input::ActionControl(Input::ActionType::MOUSE,    Input::MOUSE_WHEEL_UP);
    const auto inputMWDown = Input::ActionControl(Input::ActionType::MOUSE,    Input::MOUSE_WHEEL_DOWN);
    const auto inputLALT   = Input::ActionControl(Input::ActionType::KEYBOARD, SDL_SCANCODE_LALT);
    const auto inputPlus   = Input::ActionControl(Input::ActionType::KEYBOARD, SDL_SCANCODE_EQUALS);
    const auto inputMinus  = Input::ActionControl(Input::ActionType::KEYBOARD, SDL_SCANCODE_MINUS);
    Input::Action panAction     {"Pan",      {inputMMB,    inputLALT}};
    Input::Action zoominAction  {"Zoom In",  {inputMWUp,   inputPlus}};
    Input::Action zoomoutAction {"Zoom Out", {inputMWDown, inputMinus}};

    auto view = Renderer2D::getView();
    debugCamera(view, 0.1f, 40.f, 0.15f, &panAction, &zoominAction, &zoomoutAction);
    Renderer2D::setView(view);

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

void update()
{
    gridMapInput();

    // Diag UI
    {
        beginDiagWidgetExt();
        static int mapSizeInput[2] ={ map.width(), map.height() };
        if (ImGui::InputInt2("Map Size", mapSizeInput,
            ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsNoBlank))
        {
            map.resize(Vector2i(mapSizeInput[0], mapSizeInput[1]));
        }
        ImGui::Checkbox("Draw Node Tree", &drawNodeTree);
        ImGui::Checkbox("Draw Node Cost (Perf Heavy)", &drawNodeCost);
        ImGui::Checkbox("Draw Raycast", &drawRayCast);
        ImGui::Checkbox("Draw Grid Icons", &drawGridIcons);
        if (ImGui::SliderFloat("Diagonal Cost", &diagonalCost, 0.0f, 3.f)) { refreshPath(); }
        if (ImGui::Checkbox("Allow Diagonals", &allowDiagonals)) { refreshPath(); } 
        ImGui::End();
    }
}

void init()
{
    iconFont.loadFromFile("assets/roboto.ttf", 24, 0, 128, FontRenderMode::SDF,    TextureMagFilter::Linear);
    uiFont  .loadFromFile("assets/proggy.ttf", 24, 0, 128, FontRenderMode::Normal, TextureMagFilter::Linear);
    setGoal(map.size() - 1);

    Vector2f mapSizePx = Vector2f(map.size()) * gridSize;
    auto view = Renderer2D::getView();
    view.center = mapSizePx / 2.f;
    Renderer2D::setView(view);
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
    Input::init(window);
    Renderer::create();
    Renderer2D::create();
    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    init();

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

        update();

        Renderer::clearColor();
        drawMap();
        Renderer2D::render();

        String controlsText = R"(
           Middle Mouse/LAlt       : Pan Camera
           Scroll Wheel/Plus/Minus : Zoom In/Out
            Left Click             : Add Obstacle
            Right Click            : Remove Obstacle
    Shift + Left Click             : Set Start Position
    Shift + Right Click            : Set End Position
)";

        Renderer2D::drawText(controlsText, uiFont, Vector2f(20, -10));
        Renderer2D::render();

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}