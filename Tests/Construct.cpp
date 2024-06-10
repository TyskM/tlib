
// TODO: Finish model example
#include <TLib/Types/Types.hpp>
#include <TLib/Media/Renderer.hpp>
#include <TLib/Media/View.hpp>
#include <TLib/RNG.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Containers/Span.hpp>
#include <TLib/Media/Resource/Asset.hpp>
#include <TLib/thirdparty/ImGuiColorTextEdit/TextEditor.h>
#include "Common.hpp"

#include <TLib/Physics3D.hpp>
#include <TLib/Media/Renderer3D.hpp>
#include <TLib/ECS/Scene.hpp>

using namespace Physics3D;

void imGuiVectorFloatEdit(const String& label, Vector<float>& v,
    float vmin, float vmax, const char* format = "%.3f", ImGuiSliderFlags flags = 0)
{
    ImGui::BeginGroup();

    ImGui::SeparatorText(label.c_str());
    ImGui::Indent();
    float itemWidth = ImGui::GetWindowSize().x / 2.5f;
    ImGui::PushItemWidth(itemWidth);

    size_t i = 0;
    if (ImGui::Button("+##prepend"))
    { v.insert(v.begin(), 0); }
    for (auto& value : v)
    {
        if (ImGui::Button(String("-##" + std::to_string(i)).c_str()))
        { v.erase(v.begin() + i); break; }
        ImGui::SameLine();
        ImGui::SliderFloat(std::to_string(i).c_str(), &value, vmin, vmax, format, flags);
        ImGui::SameLine();
        if (ImGui::Button (String("+##" + std::to_string(i)).c_str()))
        { v.insert(v.begin() + i + 1, 0); }
        ++i;
    }

    ImGui::PopItemWidth();
    ImGui::Unindent();
    ImGui::EndGroup();
}

using R3D = Renderer3D;

struct ModelInstance
{
    Transform3D transform;
    Mesh*       modelPtr = nullptr;
};

Vector<Vector3f> lines;

static void addForceAtPosInternal(PxRigidBody& body, const PxVec3& force, const PxVec3& pos, PxForceMode::Enum mode, bool wakeup)
{
/*	if(mode == PxForceMode::eACCELERATION || mode == PxForceMode::eVELOCITY_CHANGE)
    {
        Ps::getFoundation().error(PxErrorCode::eINVALID_PARAMETER, __FILE__, __LINE__,
            "PxRigidBodyExt::addForce methods do not support eACCELERATION or eVELOCITY_CHANGE modes");
        return;
    }*/

    const PxTransform globalPose = body.getGlobalPose();
    const PxVec3 centerOfMass = globalPose.transform(body.getCMassLocalPose().p);

    const PxVec3 torque = (pos - centerOfMass).cross(force);
    body.addForce(force, mode, wakeup);
    body.addTorque(torque, mode, wakeup);
}

static void addForceAtLocalPos(PxRigidBody& body, const PxVec3& force, const PxVec3& pos, PxForceMode::Enum mode, bool wakeup=true)
{
    //transform pos to world space
    const PxVec3 globalForcePos = body.getGlobalPose().transform(pos);

    addForceAtPosInternal(body, force, globalForcePos, mode, wakeup);
}

static void defaultCCTInteraction(const PxControllerShapeHit& hit)
{
    // https://github.com/NVIDIAGameWorks/PhysX-3.4/blob/master/PhysX_3.4/Samples/SampleCCTSharedCode/SampleCCTActor.cpp#L256
    PxRigidDynamic* actor = hit.actor->is<PxRigidDynamic>();
    if (actor)
    {
        if (actor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC)
            return;

        if (0)
        {
            const PxVec3 p = actor->getGlobalPose().p + hit.dir * 10.0f;

            PxShape* shape;
            actor->getShapes(&shape, 1);
            PxRaycastHit newHit;
            PxU32 n = PxShapeExt::raycast(*shape, *shape->getActor(), p, -hit.dir, 20.0f, PxHitFlag::ePOSITION, 1, &newHit);
            if (n)
            {
                // We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
                // useless stress on the solver. It would be possible to enable/disable vertical pushes on
                // particular objects, if the gameplay requires it.
                const PxVec3 upVector = hit.controller->getUpDirection();
                const PxF32 dp = hit.dir.dot(upVector);
                // shdfnd::printFormatted("%f\n", fabsf(dp));
                if (fabsf(dp)<1e-3f)
                // if(hit.dir.y==0.0f)
                {
                    const PxTransform globalPose = actor->getGlobalPose();
                    const PxVec3 localPos = globalPose.transformInv(newHit.position);
                    addForceAtLocalPos(*actor, hit.dir*hit.length*1000.0f, localPos, PxForceMode::eACCELERATION);
                }
            }
        }

        // We only allow horizontal pushes. Vertical pushes when we stand on dynamic objects creates
        // useless stress on the solver. It would be possible to enable/disable vertical pushes on
        // particular objects, if the gameplay requires it.
        const PxVec3 upVector = hit.controller->getUpDirection();
        const PxF32 dp = hit.dir.dot(upVector);
        // shdfnd::printFormatted("%f\n", fabsf(dp));
        if (fabsf(dp)<1e-3f)
        // if(hit.dir.y==0.0f)
        {
            const PxTransform globalPose = actor->getGlobalPose();
            const PxVec3 localPos = globalPose.transformInv(toVec3(hit.worldPos));
            addForceAtLocalPos(*actor, hit.dir*hit.length*1000.0f, localPos, PxForceMode::eACCELERATION);
        }
    }
}

struct PlayerController : PxControllerBehaviorCallback, PxUserControllerHitReport
{
public:
    Vector2f moveDir;
    Vector3f up = Vector3f::up();

    // Input
    bool jump    = false;
    bool primary = false;

    View3D   camera;
    Quat     bodyRot;
    bool     freeCam      = false;
    float    sens         = 0.1f;
    float    freeCamSpeed = 12.f;
    float    yaw          = 0.f;
    float    pitch        = 0.f;

    float maxPitch =  89.f;
    float minPitch = -89.f;

    float    moveSpeed =  0.01f;
    float    gravity   = -0.011f;
    float    friction  =  0.92f;
    float    jumpPower =  0.25f;
    Vector3f velocity;

    PxController* playerController = nullptr;
    float         playerHeight     = 0.8f;

    bool isOnGround = false;
    bool lookingAtGeometry = false;

    Vector3f getPos() const
    {
        return Vector3f(playerController->getPosition());
    }

    void update(float delta, bool mouseCaptured)
    {
        if (mouseCaptured)
        {
            yaw   += float(Input::mouseDelta.x) * sens;
            pitch += float(Input::mouseDelta.y) * sens;
            pitch  = std::clamp(pitch, minPitch, maxPitch);
        }

        moveDir.x = Input::isKeyPressed(SDL_SCANCODE_D) - Input::isKeyPressed(SDL_SCANCODE_A);
        moveDir.y = Input::isKeyPressed(SDL_SCANCODE_W) - Input::isKeyPressed(SDL_SCANCODE_S);

        auto yawRad   = glm::radians(yaw);
        auto pitchRad = glm::radians(pitch);

        Quat qPitch  (pitchRad, Vector3f(1, 0, 0));
        Quat qYaw    (yawRad, Vector3f(0, 1, 0));
        camera.rot  = (qPitch * qYaw).normalized();
        bodyRot     = qYaw;

        // Camera Look
        auto forward = camera.rot.forward();
        if (freeCam)
        {
            Vector3f finalMovement{};
            if (moveDir.x) finalMovement += forward.cross(up).normalized() * moveDir.x * delta;
            if (moveDir.y) finalMovement += forward * moveDir.y * delta;

            camera.pos += finalMovement * freeCamSpeed;
        }
        else
        {
            const auto& pos = playerController->getPosition();
            camera.pos = Vector3f(pos.x, pos.y + playerHeight/2.f, pos.z);
        }

        if (!freeCam && Input::isKeyJustPressed(SDL_SCANCODE_SPACE))
        { jump = true; }

        if (Input::isMouseJustPressed(Input::MOUSE_LEFT))
        { primary = true; }

    }

    void fixedUpdate(float delta)
    {
        if (freeCam) {}
        else
        {
            const auto& pos = playerController->getPosition();
            PxControllerState state;
            playerController->getState(state);

            Vector3f fw = bodyRot.forward();
            if (moveDir.x) velocity += fw.cross(up).normalized() * moveDir.x * moveSpeed;
            if (moveDir.y) velocity += fw * moveDir.y * moveSpeed;

            auto* scene = playerController->getScene();

            PxRaycastBuffer hit;
            PxQueryFilterData d; d.flags = PxQueryFlag::eSTATIC;
            lookingAtGeometry = false; //scene->raycast(PxVec3(camera.pos.x, camera.pos.y, camera.pos.z), PxVec3(forward.x, forward.y, forward.z), 10000.f, hit, PxHitFlag::eDEFAULT, d);

            if (primary)
            {
                primary = false;
                if (lookingAtGeometry)
                {
                    lines.emplace_back(camera.pos);
                    lines.emplace_back(hit.block.position.x, hit.block.position.y, hit.block.position.z);
                }
            }

            if (jump)
            {
                jump = false;
                if (isOnGround)
                { velocity.y += jumpPower; isOnGround = false; }
            }
            else if (!isOnGround)
            { velocity.y += gravity; }
            else
            { velocity.y = gravity; }

            velocity.x *= friction;
            velocity.z *= friction;

            PxControllerFilters filters;
            filters.mFilterFlags = PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC;
            PxControllerCollisionFlags flags = playerController->move(PxVec3(velocity.x, velocity.y, velocity.z), 0.002f, delta, filters);
            isOnGround = flags & PxControllerCollisionFlag::eCOLLISION_DOWN;
        }
    }

    void resetCamera()
    {
        camera     = View3D();
        pitch      = 0;
        yaw        = 0;
    }

    /**
    \brief Retrieve behavior flags for a shape.
    When the CCT touches a shape, the CCT's behavior w.r.t. this shape can be customized by users.
    This function retrieves the desired PxControllerBehaviorFlag flags capturing the desired behavior.
    \note See comments about deprecated functions at the start of this class
    \param[in] shape	The shape the CCT is currently touching
    \param[in] actor	The actor owning the shape
    \return Desired behavior flags for the given shape */
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxShape& shape, const PxActor& actor) override
    {
        return PxControllerBehaviorFlags(0);
    }

    /**
    \brief Retrieve behavior flags for a controller.
    When the CCT touches a controller, the CCT's behavior w.r.t. this controller can be customized by users.
    This function retrieves the desired PxControllerBehaviorFlag flags capturing the desired behavior.
    \note The flag PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT is not supported.
    \note See comments about deprecated functions at the start of this class
    \param[in] controller	The controller the CCT is currently touching
    \return Desired behavior flags for the given controller */
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxController& controller) override
    {
        return PxControllerBehaviorFlags(0);
    }

    /**
    \brief Retrieve behavior flags for an obstacle.
    When the CCT touches an obstacle, the CCT's behavior w.r.t. this obstacle can be customized by users.
    This function retrieves the desired PxControllerBehaviorFlag flags capturing the desired behavior.
    \note See comments about deprecated functions at the start of this class
    \param[in] obstacle		The obstacle the CCT is currently touching
    \return Desired behavior flags for the given obstacle */
    virtual PxControllerBehaviorFlags getBehaviorFlags(const PxObstacle& obstacle) override
    {
        return PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT | PxControllerBehaviorFlag::eCCT_SLIDE;
    }

    /**
    \brief Called when current controller hits a shape.
    This is called when the CCT moves and hits a shape. This will not be called when a moving shape hits a non-moving CCT.
    \param[in] hit Provides information about the hit. */
    virtual void onShapeHit(const PxControllerShapeHit& hit) override
    {
        defaultCCTInteraction(hit);
    }

    /**
    \brief Called when current controller hits another controller.
    \param[in] hit Provides information about the hit. */
    virtual void onControllerHit(const PxControllersHit& hit) override
    {

    }

    /**
    \brief Called when current controller hits a user-defined obstacle.
    \param[in] hit Provides information about the hit. */
    virtual void onObstacleHit(const PxControllerObstacleHit& hit) override
    {

    }
};

Window     window;
MyGui      imgui;
FPSLimit   fpslimit;
Timer      deltaTimer;

TextEditor vertShaderEdit;
TextEditor fragShaderEdit;

bool debugDrawPhysics = false;


const char* presetModelPaths[7] =
{
    "assets/primitives/cube.obj",
    "assets/sponza/sponza.obj",
    "assets/backpack/backpack.obj",
    "assets/primitives/cylinder.obj",
    "assets/primitives/monkey.obj",
    "assets/primitives/sphere.obj",
    "assets/primitives/torus.obj"
};
int guiSelectedModel = 0;
String guiModel;

ColorRGBAf lightColor ("#FFE082");
ColorRGBAf sunColor   ("#6561FF");
ColorRGBAf spotColor  ("#FFC182");
Vector3f   sunDir     = Vector3f(-0.225, -1.f, 1.f);
Vector3f   sunInterp  = Vector3f(0.f, 0.f, 0.f);
float      lightPower = 1.f;
bool       sunEnabled = true;
float      sunPower   = 1.f;
float      spotPower  = 200.f;
float      spotAngle  = 10.f;
float      spotOuterAngle = 30.f;
bool       ambientColorSkySync    = true;
bool       ambientStrSkyValueSync = true;

R3D::Frustum frustum;
bool frustumSet = false;

float totalTime = 0.f;

PxPhysics* physics                 = nullptr;
PxScene*   physicsScene            = nullptr;
PxControllerManager* controllerMan = nullptr;

Scene            scene;
Entity           level;
PlayerController controller;

void init()
{
    auto lang = TextEditor::LanguageDefinition::GLSL();
    vertShaderEdit.SetLanguageDefinition(lang);
    fragShaderEdit.SetLanguageDefinition(lang);
    vertShaderEdit.SetText(myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString());
    fragShaderEdit.SetText(myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString());

    scene.init();
    physics      = scene.phys3d.phys;
    physicsScene = scene.phys3d.scene;

    const Path theHolyCubeModel("assets/primitives/cube.obj");
    for (size_t i = 1; i < 40; i+=2)
    {
        Vector3f scale = Vector3f(0.1f, 0.1f, 0.1f) * (int)i/2;

        auto theHolyCube = scene.createEntity();
        emplaceComponent<Transform3D>(theHolyCube).scale = scale;
        emplaceComponent<MeshInstance3D>(theHolyCube, theHolyCubeModel);

        auto& theHolyCubeBody = emplaceComponent<Physics3DBody>(theHolyCube);
        theHolyCubeBody.init(scene.phys3d);
        theHolyCubeBody.makeRigidBody();
        theHolyCubeBody.addBoxCollider(scale);
        theHolyCubeBody.setPosition(Vector3f(0.f, 50.f, i));
    }

    // Load level mesh
    MeshData levelMesh;
    levelMesh.loadFromFile("assets/scuffed_construct.glb");
    level = scene.createEntity();

    emplaceComponent<Transform3D>(level);
    emplaceComponent<MeshInstance3D>(level, "assets/scuffed_construct.glb");

    auto& body = emplaceComponent<Physics3DBody>(level);
    body.init(scene.phys3d);
    body.makeStaticBody();
    body.addTriMeshCollider(levelMesh);


    // Level collision
    //for (auto& submesh : levelMesh.subMeshes)
    //{
    //    PxTriangleMeshDesc meshDesc;
    //
    //    Vector<Vector3f> vertices;
    //    for (auto& vert : submesh.vertices)
    //    {
    //        vertices.emplace_back(vert.position);
    //    }
    //
    //    meshDesc.points.count           = vertices.size();;
    //    meshDesc.points.stride          = sizeof(Vector3f);
    //    meshDesc.points.data            = vertices.data();
    //    meshDesc.triangles.count        = submesh.indices.size();
    //    meshDesc.triangles.stride       = 3*sizeof(uint32_t);
    //    meshDesc.triangles.data         = submesh.indices.data();
    //
    //    PxDefaultMemoryOutputStream writeBuffer;
    //    bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, NULL);
    //    ASSERT(status);
    //
    //    PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
    //
    //    PxTriangleMesh* triMesh = physics->createTriangleMesh(readBuffer);
    //    
    //    PxMeshScale triScale(physx::PxVec3(1.0f, 1.0f, 1.0f), physx::PxQuat(physx::PxIdentity));
    //    PxRigidStatic* rigidStatic = physics->createRigidStatic(PxTransform({0, 0, 0}));
    //    {
    //        PxShape* shape = physics->createShape(PxTriangleMeshGeometry(triMesh, triScale), *mat, true, shapeFlags);
    //        shape->setContactOffset(0.002f);
    //        shape->setRestOffset(0.002f);
    //        rigidStatic->attachShape(*shape);
    //        shape->release(); // this way shape gets automatically released with actor
    //    }
    //
    //    physicsScene->addActor(*rigidStatic);
    //}

    // Upload to GPU
    //auto& m = models.emplace_back();
    //m.modelPtr = new Mesh();
    //m.modelPtr->loadFromMemory(levelMesh);

    // Box
    //PxMaterial* mat = physics->createMaterial(1, 1, 1);
    //PxShape* boxGeom = physics->createShape(PxBoxGeometry(1.f, 1.f, 1.f), *mat, false, shapeFlags);
    //PxRigidDynamic* boxBody = physics->createRigidDynamic({ 3.f, 3.f, 0.f });
    //boxBody->attachShape(*boxGeom);

    // Player geometry
    PxMaterial* playerMat = physics->createMaterial(0, 0, 0);
    controllerMan = PxCreateControllerManager(*physicsScene);
    PxCapsuleControllerDesc cdesc;
    cdesc.material         = playerMat;
    cdesc.radius           = 0.5f;
    cdesc.height           = controller.playerHeight;
    cdesc.contactOffset    = 0.01f;
    cdesc.slopeLimit       = 0.707; // http://www2.clarku.edu/faculty/djoyce/trig/cosines.html
    cdesc.stepOffset       = 0.2f;
    cdesc.climbingMode     = PxCapsuleClimbingMode::eCONSTRAINED;
    cdesc.nonWalkableMode  = PxControllerNonWalkableMode::ePREVENT_CLIMBING_AND_FORCE_SLIDING;
    cdesc.density          = 1000.f;
    cdesc.behaviorCallback = &controller;
    cdesc.reportCallback   = &controller;
    controller.playerController = controllerMan->createController(cdesc);
    playerMat->release();
}

void shutdown()
{
    // Have to call this explicitly bc physx destructors get called first for some reason.
    scene.reset(); // TODO: fix this
}

void fixedUpdate(float delta)
{
    controller.fixedUpdate(delta);
}

void update(float delta)
{
    // Fixed update
    const  float fixedTimeStep  = 1.f/60.f;
    static float time           = 0;
    static float lastUpdateTime = 0;
    static float timeBuffer     = 0;
    time          += delta;
    timeBuffer    += time - lastUpdateTime;
    lastUpdateTime = time;
    while (timeBuffer >= fixedTimeStep)
    {
        fixedUpdate(fixedTimeStep);
        timeBuffer -= fixedTimeStep;
    }

    scene.update(delta);

    #pragma region UI
    beginDiagWidgetExt();

    int headerFlags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FramePadding;
    ImVec2 dummySize(10, 10);

    if (ImGui::CollapsingHeader("Character & Camera", headerFlags))
    {
        ImGui::Indent();
        ImGui::Checkbox("Physics Debug", &debugDrawPhysics);

        ImGui::Checkbox("Freecam", &controller.freeCam);

        ImGui::SliderFloat("Move Speed", &controller.moveSpeed, 0.f, 100.f);
        ImGui::SliderFloat("Gravity", &controller.gravity, -2.f, 2.f);
        ImGui::SliderFloat("Friction", &controller.friction, 0.f, 1.f);
        ImGui::SliderFloat("Jump Power", &controller.jumpPower, 1.f, 100.f);
        ImGui::Checkbox   ("On Ground", &controller.isOnGround);

        ImGui::Text(fmt::format("Velocity: {}", controller.velocity.toString()).c_str());
        ImGui::Text(fmt::format("Looking At Geometry: {}", controller.lookingAtGeometry).c_str());
        //ImGui::Text(fmt::format("Dir:      {}", Vector3f(controller.forward).toString()).c_str());
        //ImGui::Text(fmt::format("Dir No Y: {}", Vector3f(controller.forwardNoY).toString()).c_str());

        Vector3f playerPos(controller.getPos());
        static Vector3f guiTeleport;
        ImGui::InputFloat3("##teleportinput", &guiTeleport.x);
        if (ImGui::Button("Teleport"))
        { controller.playerController->setPosition({ guiTeleport.x, guiTeleport.y, guiTeleport.z }); }
        ImGui::SameLine();
        if (ImGui::Button("Save Pos"))
        { guiTeleport = playerPos; }
        ImGui::Text(fmt::format("Current Pos: {}", playerPos.toString()).c_str());

        static Vector3f guiGravity;
        ImGui::InputFloat3("##gravinput", &guiGravity.x);
        if (ImGui::Button("Set Gravity"))
        { physicsScene->setGravity({ guiGravity.x, guiGravity.y, guiGravity.z }); }

        ImGui::SeparatorText("Camera");
        if (ImGui::Button("Reset Camera"))
        { controller.resetCamera(); }
        ImGui::SliderFloat("FOV", &controller.camera.fov, 70.f, 170.f);

        ImGui::Text("Press ALT to toggle cursor.");
        ImGui::SliderFloat("Sensitivity", &controller.sens, 0.01f, 2.f);
        ImGui::SliderFloat("Camera Speed", &controller.freeCamSpeed, 0.01f, 400.f);
        ImGui::Text(fmt::format("Pitch : {}", controller.pitch).c_str());
        ImGui::Text(fmt::format("Yaw   : {}", controller.yaw).c_str());
        ImGui::Unindent();
    }
    // Model loading input
    //ImGui::SeparatorText("Model");
    //if (ImGui::Combo("Models", &guiSelectedModel, presetModelPaths, std::size(presetModelPaths), -1))
    //{ /*model.loadFromFile(presetModelPaths[guiSelectedModel]);*/ }
    //if (ImGui::Button("Load Custom Model"))
    //{
    //    Path p = openSingleFileDialog();
    //    if (!p.empty()) { model.loadFromFile(p); }
    //}

    if (ImGui::CollapsingHeader("Graphics", headerFlags))
    {
        ImGui::Indent();
        if (ImGui::CollapsingHeader("Lights", headerFlags))
        {
            ImGui::Indent();
            ImGui::ColorEdit4 ("Sky Color",              &R3D::skyColor.r);
            ImGui::SliderFloat("Ambient Light Strength", &R3D::ambientLightStrength, 0.f, 1.f);
            ImGui::SliderFloat("Ambient Color Factor",   &R3D::ambientColorFactor, 0.f, 1.f);
            ImGui::ColorEdit3 ("Ambient Color",          &R3D::ambientColor.r);
            ImGui::Checkbox   ("Ambient Color Sky Sync", &ambientColorSkySync);
            ImGui::Checkbox   ("Ambient Color Str Sky Value Sync", &ambientStrSkyValueSync);
            ImGui::ColorEdit3 ("Player Light",           &lightColor.r);
            ImGui::DragFloat  ("Player Light Power",     &lightPower);
            ImGui::Checkbox   ("Sun Enabled",            &sunEnabled);
            ImGui::ColorEdit3 ("Sun Light",              &sunColor.r);
            ImGui::DragFloat3 ("Sun Dir",                &sunDir.x,    0.005f, -1.f, 1.f);
            ImGui::DragFloat3 ("Sun Interp",             &sunInterp.x, 0.001f, -1.f, 1.f);
            ImGui::DragFloat  ("Sun Power",              &sunPower);
            ImGui::ColorEdit3 ("Spot Light",             &spotColor.r);
            ImGui::DragFloat  ("Spot Power",             &spotPower);
            ImGui::DragFloat  ("Spot Angle",             &spotAngle);
            ImGui::DragFloat  ("Spot Outer Angle",       &spotOuterAngle);
            ImGui::Dummy(dummySize);
            ImGui::Unindent();
        }
        if (ImGui::CollapsingHeader("View Distance", headerFlags))
        {
            ImGui::Indent();
            ImGui::Checkbox   ("Fog", &R3D::fog);
            ImGui::ColorEdit3 ("Fog Color", &R3D::fogColor.r);
            ImGui::SliderFloat("Near Plane", &controller.camera.znear, 0.f, 1.f);
            ImGui::SliderFloat(" Far Plane", &controller.camera.zfar, 0.f, 500.f);
            ImGui::Dummy(dummySize);
            ImGui::Unindent();
        }
        if (ImGui::CollapsingHeader("Shadows", headerFlags))
        {
            ImGui::Indent();
            ImGui::Checkbox   ("Enabled",   &R3D::shadows);
            auto ret = imguiEnumCombo("Face Cull Mode##shadows", R3D::shadowFaceCullMode);
            if (ret.first) { R3D::shadowFaceCullMode = ret.second; }

            uint32_t minRes = 1024;
            uint32_t maxRes = 1024*8;
            if (ImGui::SliderScalar("Resolution", ImGuiDataType_U32, &R3D::shadowSize, &minRes, &maxRes, 0, ImGuiSliderFlags_AlwaysClamp))
            R3D::shadowSize = math::stepifyRound<uint32_t>(R3D::shadowSize, 1024);

            ImGui::SliderFloat("Frustum Z Multiplier", &R3D::shadowFrustZMult, 0.1f, 20.f);
            ImGui::SliderFloat("Distance",  &R3D::shadowDistance, 1.f, 200.f);
            ImGui::SliderInt  ("PCF Steps", &R3D::shadowPcfSteps, 0, 6);
            ImGui::SliderFloat("Min Bias",  &R3D::minShadowBias, 0, 2.f);
            ImGui::SliderFloat("Max Bias",  &R3D::maxShadowBias, 0, 2.f);
            imGuiVectorFloatEdit("CSM Breakpoints", R3D::cascadeBreakpoints, 0.1f, R3D::camera.zfar);
            ImGui::Dummy(dummySize);
            ImGui::Unindent();
        }

        ImGui::Checkbox("MSAA", &R3D::useMSAA);

        auto ret = imguiEnumCombo("Face Cull Mode", R3D::faceCullMode);
        if (ret.first) { R3D::faceCullMode = ret.second; }

        if (ImGui::Button("Snapshot Frustum"))
        {
            frustum = R3D::getCurrentCameraFrustum();
            frustumSet = true;
        }

        int textEditFlags = ImGuiInputTextFlags_AllowTabInput;

        if (ImGui::Button("Compile Shaders"))
        { R3D::setPBRShader(vertShaderEdit.GetText(), fragShaderEdit.GetText()); }
        if (ImGui::CollapsingHeader("Vert Shader", headerFlags))
        {
            vertShaderEdit.Render("Vertex Shader Edit", ImVec2(0, 720), true);
        }
        if (ImGui::CollapsingHeader("Frag Shader", headerFlags))
        {
            fragShaderEdit.Render("Fragment Shader Edit", ImVec2(0, 720), true);
        }

        if (ImGui::CollapsingHeader("CSMs", headerFlags))
        {
            for (auto& map : R3D::csmTextures)
            {
                ImGui::Image(*map, Vector2f(128.f, 128.f) * 4.f);
            }
        }

        ImGui::Dummy(dummySize);
        ImGui::Unindent();
    }
    ImGui::End();
    #pragma endregion

    // Camera
    {
        if (Input::isKeyJustPressed(SDL_SCANCODE_LALT))
        { window.toggleFpsMode(); }

        controller.update(delta, window.getFpsMode());

        R3D::setCamera(controller.camera);
    }

    if (ambientColorSkySync)
    {
        R3D::ambientColor = ColorRGBf(R3D::skyColor);
    }
    if (ambientStrSkyValueSync)
    {
        auto skyColorHsv = ColorHSVf(ColorRGBf(R3D::skyColor));
        R3D::ambientLightStrength = skyColorHsv.v;
    }

    sunDir += sunInterp * delta;
    sunDir.x = std::clamp(sunDir.x, -1.f, 1.f);
    sunDir.y = std::clamp(sunDir.y, -1.f, 1.f);
    sunDir.z = std::clamp(sunDir.z, -1.f, 1.f);
}

void draw(float delta)
{
    //sunDir = Vector3f::forward() * controller.camera.rot;
    Vector3f lightPos  = controller.getPos();
    R3D::addPointLight(lightPos, ColorRGBf(lightColor), lightPower);

    if (sunEnabled)
        R3D::addDirectionalLight(sunDir, ColorRGBf(sunColor), sunPower);

    //R3D::addSpotLight(lightPos, Vector3f(controller.forward), spotAngle, spotOuterAngle, ColorRGBf(spotColor), spotPower);

    Vector3f soffset = Vector3f::up() * 4.f + Vector3f::right() * 4;
    R3D::drawLines(std::initializer_list{ soffset, soffset + sunDir*2.f }, sunColor, GLDrawMode::Lines);

    if (frustumSet)
    {
        Vector<Vector3f> frustLines(24);

        Vector3f nearBottomLeft  (frustum.corners[0]);
        Vector3f nearTopLeft     (frustum.corners[2]);
        Vector3f nearTopRight    (frustum.corners[6]);
        Vector3f nearBottomRight (frustum.corners[4]);
        Vector3f farBottomLeft   (frustum.corners[1]);
        Vector3f farTopLeft      (frustum.corners[3]);
        Vector3f farTopRight     (frustum.corners[7]);
        Vector3f farBottomRight  (frustum.corners[5]);

        frustLines.push_back(nearBottomLeft);
        frustLines.push_back( farBottomLeft);
        frustLines.push_back(nearTopLeft);
        frustLines.push_back( farTopLeft);
        frustLines.push_back(nearTopRight);
        frustLines.push_back( farTopRight);
        frustLines.push_back(nearBottomRight);
        frustLines.push_back(farBottomRight);

        frustLines.push_back(farBottomLeft);
        frustLines.push_back(farBottomRight);
        frustLines.push_back(farBottomRight);
        frustLines.push_back(farTopRight);
        frustLines.push_back(farTopRight);
        frustLines.push_back(farTopLeft);
        frustLines.push_back(farTopLeft);
        frustLines.push_back(farBottomLeft);

        frustLines.push_back(nearBottomLeft);
        frustLines.push_back(nearBottomRight);
        frustLines.push_back(nearBottomRight);
        frustLines.push_back(nearTopRight);
        frustLines.push_back(nearTopRight);
        frustLines.push_back(nearTopLeft);
        frustLines.push_back(nearTopLeft);
        frustLines.push_back(nearBottomLeft);

        R3D::drawLines(frustLines, ColorRGBAf::white(), GLDrawMode::Lines);
    }

    scene.render(delta);

    R3D::drawLines(std::initializer_list{ Vector3f::up()      * 10, Vector3f() }, ColorRGBAf::green(), GLDrawMode::Lines);
    R3D::drawLines(std::initializer_list{ Vector3f::right()   * 10, Vector3f() }, ColorRGBAf::red(),   GLDrawMode::Lines);
    R3D::drawLines(std::initializer_list{ Vector3f::forward() * 10, Vector3f() }, ColorRGBAf::blue(),  GLDrawMode::Lines);
    R3D::drawLines(lines, ColorRGBAf::green(), GLDrawMode::Lines);

    if (debugDrawPhysics)
    {
        scene.phys3d.debugDraw();
    }
    
}

int main()
{
    WindowCreateParams p;
    p.size  = { 1600, 900 };
    p.title = "Construct";
    window.create(p);
    Renderer::create();

    Renderer2D::create();
    
    R3D::create();

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
        if (!(io.WantCaptureMouse)) { Input::updateMouse(); }

        imgui.newFrame();
        ImGui::BeginDisabled(window.getFpsMode());

        Vector2f mwpos = Renderer2D::getMouseWorldPos();

        glClear(GL_DEPTH_BUFFER_BIT);
        totalTime += delta;
        update(delta);
        draw(delta);
        R3D::render();
        Renderer2D::render();
        drawDiagWidget(&fpslimit);
        ImGui::EndDisabled();
        imgui.render();
        window.swap();
        fpslimit.wait();
    }

    shutdown();

    return 0;
}