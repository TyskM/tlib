#pragma once

/*
TODO: Save/Load project
TODO: Undo/Redo. Use command pattern ez
*/

#include <TLib/Media/AutoTile.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer2D.hpp>
#include <TLib/Media/Platform/Input.hpp>
#include <TLib/Media/Platform/Window.hpp>
#include <TLib/Media/Platform/FPSLimit.hpp>
#include <TLib/Timer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/Media/ImGuiWidgets.hpp>
#include <TLib/Files.hpp>
#include <TLib/Containers/Bitset.hpp>

#include <nlohmann/json.hpp>
using namespace nlohmann;

struct TilesetAppTile
{
    Bitset<8, uint8_t> bits;
    bool               enabled = false;
};

struct TilesetAppProject
{
    Vector2i size      = { 128, 128 };
    Vector2i cellSize  = { 16,  16  };
    Vector2i cellCount = size / cellSize;
    Path     filePath;
    GridMap2D<TilesetAppTile> tileData;

    // Do Not Serialize
    Texture texture;
};

struct TilesetPorterCtx
{
    const UnorderedMap<Vector2i, uint8_t> dirToBitMap =
    {
        { Vector2i( 0,  1), 0 },
        { Vector2i( 1,  1), 1 },
        { Vector2i( 1,  0), 2 },
        { Vector2i( 1, -1), 3 },
        { Vector2i( 0, -1), 4 },
        { Vector2i(-1, -1), 5 },
        { Vector2i(-1,  0), 6 },
        { Vector2i(-1,  1), 7 }
    };

    TilesetAppProject proj;
    Vector2i hoveredCell;
    Vector2i hoveredDir;
    Vector2f mousePos;
    bool     mouseInBounds   = false;
    bool     validBitHovered = false;
    bool     showBorder = true;
    bool     showGrid   = true;

    TilesetPorterCtx()
    {
        newProj();
    }

    void exportProj()
    {
        Path exportPath = saveFileDialog("Export", fs::current_path()/proj.filePath.stem().replace_extension(".json"), {"*.json"});
        if (exportPath.empty()) { return; }
        tlog::info(exportPath.string());

        Tileset outputTileset;
        outputTileset.cellDimensions = proj.cellCount;
        outputTileset.cellSize       = proj.cellSize;
        for (int32_t x = 0; x < proj.tileData.width();  x++) {
        for (int32_t y = 0; y < proj.tileData.height(); y++)
        {
            auto& tileData = proj.tileData.at(x, y);
            if (!tileData.enabled) { continue; }
            if (outputTileset.lut.contains(*tileData.bits.data()))
            { continue; }

            outputTileset.lut[*tileData.bits.data()] = Vector2i(x, y);
        }}

        ordered_json js;
        outputTileset.to_json(js);

        writeToFile(exportPath, js.dump(4));
    }

    void importProj()
    {
        Path importPath = openSingleFileDialog("Import", fs::current_path(), {"*.json"});
        if (importPath.empty()) { return; }

        ordered_json js = ordered_json::parse(readFile(importPath), nullptr, true, true);
        Tileset importTileset;
        importTileset.from_json(js);

        proj.tileData.clear();
        proj.cellCount = importTileset.cellDimensions;
        proj.cellSize  = importTileset.cellSize;
        for(auto& [k, v] : importTileset.lut)
        {
            proj.tileData.at(v).enabled     = true;
           *proj.tileData.at(v).bits.data() = k;
        }
    }

    void newProj()
    {
        proj = TilesetAppProject();
        updateProj();
    }

    TilesetAppTile* getCellData(const Vector2i& cell)
    {
        if (
            !(math::sign(cell.x) != -1 && math::sign(cell.y) != -1) ||
            !(proj.tileData.inBounds(cell))
           )
        { return nullptr; }

        return &proj.tileData.at(cell.x, cell.y);
    }

    void setBit(const Vector2i& cell, const Vector2i& dir, bool value)
    {
        auto cellPtr = getCellData(cell);
        if (!cellPtr) { return; }

        auto& bits = cellPtr->bits;
        auto& bit  = dirToBitMap.at(dir-Vector2i(1,1));
        bits.set(bit, value);
    }

    Vector2f getMouseWorldPos()
    {
        return localToWorldPoint(Vector2f(Input::mousePos), Renderer2D::getView(), Renderer::getFramebufferSize());
    }

    void setProjTexture(const Path& path)
    {
        ASSERT(!path.empty());
        proj.texture.loadFromFile(path);
        proj.texture.setFilter(TextureMinFilter::Nearest, TextureMagFilter::Nearest);
        proj.size = proj.texture.getSize();
        updateProj();
    }

    void resizeCells(const Vector2i& newSize)
    {
        if (newSize.x <= 0 || newSize.y <= 0)
        { tlog::warn("resizeCells(newSize): newSize <= 0"); return; }

        proj.cellSize = newSize;
        updateProj();
    }

    void updateProj()
    {
        proj.cellCount = proj.size / proj.cellSize;
        proj.tileData.resize(proj.cellCount);
    }

    void update()
    {
        auto view = Renderer2D::getView();
        debugCamera(view);
        Renderer2D::setView(view);

        mousePos      = getMouseWorldPos();
        mouseInBounds = Rectf(0, 0, Vector2f(proj.size)).contains(mousePos);

        if (mouseInBounds)
        {
            hoveredCell = posToGridPos(mousePos, Vector2f(proj.cellSize));
            Vector2i hoveredGlobalBit = posToGridPos(mousePos, Vector2f(proj.cellSize)/3.f);
            hoveredDir = hoveredGlobalBit.abs().mod(3);
            validBitHovered = (hoveredDir != Vector2i(1, 1));

            if      (validBitHovered && Input::isMousePressed(Input::MOUSE_LEFT))
            {
                setBit(hoveredCell, hoveredDir, true);
            }
            else if (validBitHovered && Input::isMousePressed(Input::MOUSE_RIGHT))
            {
                setBit(hoveredCell, hoveredDir, false);
            }
            else if (Input::isMousePressed(Input::MOUSE_LEFT))
            {
                auto cellPtr = getCellData(hoveredCell);
                if (cellPtr)
                { cellPtr->enabled = true; }
            }
            else if (Input::isMousePressed(Input::MOUSE_RIGHT))
            {
                auto cellPtr = getCellData(hoveredCell);
                if (cellPtr)
                { cellPtr->enabled = false; }
            }
        }

        beginDiagWidgetExt();

        ImGui::SeparatorText("Project");
        if (ImGui::Button("Load Image"))
        {
            Path path = openSingleFileDialog("Open Image File", Path(), { "*.jpg","*.png" }, "Image Files");
            if (!path.empty())
            { setProjTexture(path); proj.filePath = path; }
        }
        if (ImGui::Button("Export"))
        { exportProj(); }
        if (ImGui::Button("Import"))
        { importProj(); }

        ImGui::SeparatorText("Edit");

        static Vector2i uiCellSize = proj.cellSize;
        ImGui::PushItemWidth(64.f);
        ImGui::InputInt2("Cell Size", &uiCellSize.x);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        if (ImGui::Button("Resize"))
        {
            resizeCells(uiCellSize);
        }

        if (ImGui::Button("Clear"))
        {
            proj.tileData.clear();
        }

        ImGui::SeparatorText("View");
        ImGui::Checkbox     ("Show Grid"  , &showGrid);
        ImGui::Checkbox     ("Show Border", &showBorder);

        ImGui::SeparatorText   ("Info");        
        ImGui::Text(fmt::format("Hovered Cell : {}", hoveredCell   .toString()).c_str());
        ImGui::Text(fmt::format("Hovered Bit  : {}", hoveredDir    .toString()).c_str());
        ImGui::Text(fmt::format("Cell Count   : {}", proj.cellCount.toString()).c_str());
        ImGui::Text(fmt::format("Image Size   : {}", proj.size     .toString()).c_str());

        ImGui::SeparatorText("Diag");

        ImGui::End();
    }

    void render()
    {
        const Vector2f fgridSize    (proj.cellSize);
        const Vector2f fhoveredCell (hoveredCell);
        const Vector2f fhoveredBit  (hoveredDir);

        if (proj.texture.created())
        {
            Rectf dstRect = Rectf(0, 0, Vector2f(proj.texture.getSize()));
            DrawTextureParams p(proj.texture, dstRect);
            Renderer2D::drawTexture(p);
        }
        if (showBorder)             { Renderer2D::drawRect(Rectf(0, 0, proj.size.x, proj.size.y), 0.f, false, ColorRGBAf::black());   }
        if (showGrid)               { Renderer2D::drawGrid(Vector2f(), proj.size / proj.cellSize, Vector2f(proj.cellSize), "0000ff"); }

        // Draw set bits
        auto size = proj.tileData.size();
        for (int32_t x = 0; x < size.x; x++) {
        for (int32_t y = 0; y < size.y; y++)
        {
            auto& tileData = proj.tileData.at(x, y);
            auto& bits     = tileData.bits;

                  Vector2f   offset = Vector2f(x, y) * fgridSize;
            const ColorRGBAf color = ColorRGBAf::red().setA(0.3f);
            const Vector2f   bitSize = fgridSize / 3.f;

            auto drawBit = [&](int x, int y)
            {
                Rectf rect(offset + bitSize * Vector2f(x, y), bitSize);
                Renderer2D::drawRect(rect, 0.f, true, color);
            };

            if (tileData.enabled) { drawBit(1, 1); }
            if (bits.test(0))     { drawBit(1, 2); }
            if (bits.test(1))     { drawBit(2, 2); }
            if (bits.test(2))     { drawBit(2, 1); }
            if (bits.test(3))     { drawBit(2, 0); }
            if (bits.test(4))     { drawBit(1, 0); }
            if (bits.test(5))     { drawBit(0, 0); }
            if (bits.test(6))     { drawBit(0, 1); }
            if (bits.test(7))     { drawBit(0, 2); }
        }}

        if (mouseInBounds)
        {
            // Draw hovered cell
            Renderer2D::drawRect(
                Rectf(
                    fhoveredCell * fgridSize,
                    fgridSize),
                0.f, false, ColorRGBAf::green());

            // Draw hovered bit
            Renderer2D::drawRect(
                Rectf(
                    fhoveredBit * (fgridSize/3.f) + fhoveredCell * fgridSize,
                    fgridSize/3.f),
                0.f, true, ColorRGBAf::red().setA(0.3f));
        }
    }
};

int main()
{
    Window     window;
    MyGui      imgui;
    FPSLimit   fpslimit;
    Timer      deltaTimer;

    WindowCreateParams p;
    p.size  = { 1280, 720 };
    p.title = "Tileset Porter";
    window.create(p);
    Input::init(window);
    Renderer::create();
    Renderer2D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    diagWindowName = "Tools";
    TilesetPorterCtx ctx;

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
        ctx.update();

        Renderer::clearColor("655561");
        ctx.render();
        Renderer2D::render();

        drawDiagWidget(&fpslimit);
        imgui.render();

        window.swap();

        fpslimit.wait();
    }

    return 0;
}
