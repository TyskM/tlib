
#include <TLib/Types/Types.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/RNG.hpp>
#include <TLib/Embed/Embed.hpp>
#include "Common.hpp"

enum class ViewMode
{
    Perspective,
    Orthographic
};

enum class CameraMode
{
    Free,
    Orbit
};

Window     window;
MyGui      imgui;
FPSLimit   fpslimit;
Timer      deltaTimer;

String vertShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString();
String fragShaderStr = myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString();

// Camera
View3D     camera;
CameraMode cameraMode = CameraMode::Orbit;
float      sens       = 0.1f;
float      camSpeed   = 12.f;
float      yaw        = 0.f;
float      pitch      = 0.f;
float      orbitDist  = 5.f;
Quat       orbitAngle;

// Mesh
Mesh        mesh;
Transform3D meshTransform;

// Light
Vector3f  sunDir   (0.f, -1.f, 0.f);
ColorRGBf sunColor (ColorRGBAf("#6EB7C9"));

void setCameraMode(CameraMode mode)
{
    switch (mode)
    {
    case CameraMode::Free:
        window.setFpsMode(true);
        break;
    case CameraMode::Orbit:
        window.setFpsMode(false);
        break;
    default: break;
    }
    cameraMode = mode;
}

void resetCamera()
{
    camera     = View3D();
    pitch      = 0;
    yaw        = 0;
    orbitDist  = 5;
    orbitAngle = {};
}

int main()
{
    WindowCreateParams p;
    p.size  = { 1280, 720 };
    p.title = "Model";
    window.create(p);
    Renderer::create();

    Renderer2D::create();
    using R3D = Renderer3D;
    R3D::create();

    imgui.create(window);
    deltaTimer.restart();
    fpslimit.setFPSLimit(144);
    fpslimit.setEnabled(true);

    const char* presetModelPaths[7] =
    {
        "assets/primitives/cube.obj",
        "assets/sponza/sponza.glb",
        "assets/backpack/backpack.obj",
        "assets/primitives/cylinder.obj",
        "assets/primitives/monkey.obj",
        "assets/primitives/sphere.obj",
        "assets/primitives/torus.obj"
    };
    int guiSelectedModel = 0;
    String guiModel;

    RELASSERTMSGBOX(mesh.loadFromFile(presetModelPaths[0]),
        "Model not found",
        fmt::format("Failed to find init model: {}", presetModelPaths[0]).c_str());

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

        ////
        imgui.newFrame();
        ImGui::BeginDisabled(window.getFpsMode());
        beginDiagWidgetExt();

        Vector2f mwpos = Renderer2D::getMouseWorldPos();

        glClear(GL_DEPTH_BUFFER_BIT);
        Renderer::clearColor();

        static float time = 0.f;
        time += delta;

        ImGui::SeparatorText("Camera");

        auto mode = cameraMode;
        imguiEnumCombo("Camera Mode", &cameraMode);
        if (mode != cameraMode) { setCameraMode(mode); }

        if (ImGui::Button("Reset Camera")) { resetCamera(); }

        ImGui::SliderFloat("FOV", &camera.fov, 70.f, 170.f);

        switch (cameraMode)
        {
        case CameraMode::Free:
        {
            ImGui::Text("Press ALT to toggle cursor.");
            ImGui::SliderFloat("Sensitivity", &sens, 0.01f, 2.f);
            ImGui::SliderFloat("Camera Speed", &camSpeed, 0.01f, 400.f);
            if (Input::isKeyJustPressed(SDL_SCANCODE_LALT))
            { window.toggleFpsMode(); }

            if (window.getFpsMode())
            {
                yaw   += float(Input::mouseDelta.x) * sens;
                pitch += float(Input::mouseDelta.y) * sens;
                pitch  = std::clamp(pitch, -89.f, 89.f);
            }

            Quat qPitch (glm::radians(pitch), Vector3f(1, 0, 0));
            Quat qYaw   (glm::radians(yaw),   Vector3f(0, 1, 0));
            camera.rot = (qPitch * qYaw).normalized();

            const Vector3f forward = camera.rot.forward();
            const Vector3f up      = Vector3f::up();

            Vector2f moveDir;
            moveDir.x = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
            moveDir.y = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);
            if (moveDir.x) { camera.pos += (forward.cross(up).normalized() * moveDir.x).normalized() * camSpeed * delta; }
            if (moveDir.y) { camera.pos += (forward * moveDir.y).normalized() * camSpeed * delta; }

            break;
        }
        case CameraMode::Orbit:
        {
            // Zoom
            float scrollDelta = Input::isMouseJustPressed(Input::MOUSE_WHEEL_UP) - Input::isMouseJustPressed(Input::MOUSE_WHEEL_DOWN);
            if (scrollDelta != 0.f)
            { orbitDist = std::clamp(orbitDist - (scrollDelta * 0.5f), 1.f, 20.f); }

            if (Input::isMousePressed(Input::MOUSE_MIDDLE))
            {
                yaw   += float(Input::mouseDelta.x);
                pitch += float(Input::mouseDelta.y);
                pitch  = std::clamp(pitch, -89.f, 89.f);
                orbitAngle = Quat::euler(yaw, pitch, 0.f);
            }

            camera.rot = orbitAngle;
            camera.setPos(-orbitAngle.forward() * orbitDist);
            
            break;
        }
        default: break;
        }

        R3D::setCamera(camera);

        const float radius = 20.0f;
        float lightX = sin(time) * radius;
        float lightZ = cos(time) * radius;
        Vector3f lightPos = { lightX, 0.f, lightZ };
        R3D::addDirectionalLight(sunDir, sunColor);

        R3D::drawModel(mesh, meshTransform);
        //Renderer2D::drawCircle(mwpos, 12.f);

        // Model loading input
        ImGui::SeparatorText("Model");
        if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
        { mesh.loadFromFile(presetModelPaths[guiSelectedModel]); }
        if (ImGui::Button("Load Custom Model"))
        {
            Path p = openSingleFileDialog();
            if (!p.empty()) { mesh.loadFromFile(p); }
        }
        
        ImGui::SeparatorText("Shaders");
        ImGui::DragFloat3("Sun Direction", &sunDir.x, 0.01f, -1.f, 1.f);
        ImGui::ColorEdit3("Sun Color",     &sunColor.r);
        ImGui::Checkbox("MSAA", &R3D::useMSAA);

        ImGui::SeparatorText("Diag");
        ImGui::End();
        ////

        R3D::render();
        Renderer2D::render();
        drawDiagWidget(&fpslimit);
        ImGui::EndDisabled();
        imgui.render();
        window.swap();
        fpslimit.wait();
    }

    return 0;
}