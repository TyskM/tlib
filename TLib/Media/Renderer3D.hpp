
#pragma once

#include <TLib/DataStructures.hpp>
#include <TLib/Embed/Embed.hpp>
#include <TLib/Containers/Stack.hpp>
#include <TLib/Containers/Variant.hpp>
#include <TLib/Media/Resource/MeshData.hpp>
#include <TLib/Media/Resource/Mesh.hpp>
#include <TLib/Media/View3D.hpp>
#include <TLib/Media/Transform3D.hpp>
#include <TLib/Media/Renderer2D.hpp>

class Renderer3D
{
public:
    static inline bool         useMSAA              = true; // Creates ugly artifacts (sometimes), find post processing solution.
    static inline FaceCullMode faceCullMode         = FaceCullMode::None;
    static inline bool         fog                  = false;
    static inline ColorRGBf    fogColor             = ColorRGBf(0.1f, 0.1f, 0.1f);

    static inline bool         shadows              = true;
    static inline float        shadowDistance       = 50.f;
    /* How far the shadow map camera is from the player camera.
    Values that are too low can make the camera clip with geometry.
    Values that are too high will make the camera too far away, and the shadow map will lose precision. */
    static inline float        shadowFrustZMult     = 10.0f;
    /* Shadow anti-aliasing/smoothing. 1-2 is a good default, values higher than 6(ish) will probably destroy performance */
    static inline int          shadowPcfSteps       = 1;
    static inline FaceCullMode shadowFaceCullMode   = FaceCullMode::None;
    /* Measured in shadow texels
       Shadow Acne <- Just Right -> Peter Panning */
    static inline float        maxShadowBias        = 0.4f;
    static inline float        minShadowBias        = 0.010f;

    static inline ColorRGBAf   skyColor             = ColorRGBAf(0.1f, 0.1f, 0.1f);
    static inline float        ambientLightStrength = 0.1f;

    static inline const uint32_t shadowCascadesCount = 3;

    struct ModelDrawCmd
    {
        Mesh*       model;
        Transform3D transform;
    };

    struct PrimitiveDrawCmd
    {
        GLDrawMode drawMode;
        Shader*    shader  = &defaultPrimitiveShader;
        uint32_t   posIndex, posSize; // Index and size for posAndCoords
        uint32_t   indIndex, indSize; // Index and size for indices
        ColorRGBAf color;
    };

    using DrawCmd = Variant<PrimitiveDrawCmd, ModelDrawCmd>;

    struct PrimVertex
    {
        Vector3f   pos;
        ColorRGBAf color;
    };

    struct DirectionalLight
    {
        Vector3f  dir;
        ColorRGBf color;
        float     power = 1.f;
    };

    struct PointLight
    {
        Vector3f  pos;
        ColorRGBf color;
        float     power = 1.f;
    };

    struct SpotLight
    {
        Vector3f  pos;
        Vector3f  dir;
        ColorRGBf color;
        float     cosAngle      = 1.f;
        float     outerCosAngle = 0.2f;
        float     power         = 1.f;
    };

    static constexpr GLuint restartIndex = std::numeric_limits<GLuint>::max();

    static inline Vector<PrimVertex> primitiveVerts;
    static inline Vector<uint32_t>   primitiveIndices;
    static inline GPUVertexData      primitiveMesh;
    static inline Shader             defaultPrimitiveShader;

    static inline Vector<DrawCmd> cmds;
    static inline View3D          camera;
    static inline Shader          shader3d;

    static inline FrameBuffer shadowFbo;
    static inline Texture     shadowTex;
    static inline uint32_t    shadowSize = 1024*4;
    static inline Shader      shadowShader;

    static inline FrameBuffer           csmFbo;
    static inline Vector<UPtr<Texture>> csmTextures;
    static inline Shader                csmShader;

    static inline uint32_t                 maxDirectionalLights = 2;
    static inline Vector<DirectionalLight> directionalLights;

    static inline uint32_t           maxPointLights = 32;
    static inline Vector<PointLight> pointLights;

    static inline uint32_t          maxSpotLights = 32;
    static inline Vector<SpotLight> spotLights;

    static void drawMeshImmediate(GPUVertexData& mesh, Texture& tex, const Transform3D& transform)
    {
        tex.bind();
        shader3d.setMat4f("model", transform.getMatrix());
        Renderer::draw(shader3d, mesh);
    }

    static void flushPrimitives(Shader& shader, GLDrawMode drawMode, Span<uint32_t> indices)
    {
        primitiveMesh.bind();
        RenderState rs;
        rs.drawMode = drawMode;
        Renderer::drawIndices(shader, indices, rs);
    }

    static Vector<Vector4f> getFrustumCornersWorldSpace(const Mat4f& proj, const Mat4f& view)
    {
        const auto inv = (proj * view).inverse();

        Vector<Vector4f> frustumCorners;
        for (unsigned int x = 0; x < 2; ++x) {
        for (unsigned int y = 0; y < 2; ++y) {
        for (unsigned int z = 0; z < 2; ++z)
        {
            const Vector4f pt =
                inv * Vector4f(2.0f * x - 1.0f,
                               2.0f * y - 1.0f,
                               2.0f * z - 1.0f, 1.0f);
            frustumCorners.push_back((pt / pt.w));
        }}}

        return frustumCorners;
    }

    struct GLSLSource
    {
    private:
        String src;

    public:
        GLSLSource() = default;
        GLSLSource(const String& str) : src{str} { }

        void inject(const String& str)
        {
            size_t versionIndex = src.find("#version");
            if (versionIndex == src.size()) { tlog::error("Missing #version"); return; }
            size_t insertPoint = src.find('\n', versionIndex) + 1;
            src.insert(insertPoint, str + '\n');
        }

        String string() const
        { return src; }
    };

    static bool created()
    {
        return shader3d.created();
    }

    static void setCamera(const View3D& camera)
    {
        // TODO: use one of those global uniform things
        Renderer3D::camera = camera;
    }

    static void drawModel(Mesh& model, const Transform3D& transform)
    {
        DrawCmd&      var = cmds.emplace_back();
        ModelDrawCmd& cmd = var.emplace<ModelDrawCmd>();
        cmd.model         = &model;
        cmd.transform     = transform;
    }

    static void drawLines(
        const Span<const Vector3f>& points,
        const ColorRGBAf&           color  = ColorRGBAf::white(),
        const GLDrawMode            mode   = GLDrawMode::LineStrip,
              Shader&               shader = defaultPrimitiveShader)
    {
        if (points.empty()) { return; }

        auto& var = cmds.emplace_back();
        auto& cmd = std::get<PrimitiveDrawCmd>(var);

        cmd.shader   = &shader;
        cmd.drawMode = mode;
        cmd.color    = color;

        cmd.indIndex = primitiveIndices.size();
        cmd.posIndex = primitiveVerts.size();
        
        cmd.indSize  = points.size();
        cmd.posSize  = points.size();

        //primitiveIndices.push_back(restartIndex);
        for (int i = 0; i < points.size(); i++)
        {
            const Vector3f& p = points[i];
            auto& vert = primitiveVerts.emplace_back();
            vert.pos   = p;
            vert.color = color;
            primitiveIndices.push_back(i);
        }
    }

    static void drawCube(const Vector3f& pos, const Vector3f& size = 1.f, const ColorRGBAf color = ColorRGBAf::white())
    {
        constexpr Vector3f vertices[8] =
        {   Vector3f(-1, -1, -1),
            Vector3f( 1, -1, -1),
            Vector3f( 1,  1, -1),
            Vector3f(-1,  1, -1),
            Vector3f(-1, -1,  1),
            Vector3f( 1, -1,  1),
            Vector3f( 1,  1,  1),
            Vector3f(-1,  1,  1) };

        constexpr int indices[36] =
        {   0, 1, 3, 3, 1, 2,
            1, 5, 2, 2, 5, 6,
            5, 4, 6, 6, 4, 7,
            4, 0, 7, 7, 0, 3,
            3, 2, 7, 7, 2, 6,
            4, 5, 0, 0, 5, 1  };

        Vector<Vector3f> lines;
        lines.reserve(36);
        for (uint32_t i = 0; i < 36; i++)
        { lines.push_back(vertices[indices[i]] * size + pos); }
        drawLines(lines, color, GLDrawMode::Triangles);
    }

    // Used for things like sun light
    static void addDirectionalLight(const Vector3f& direction, const ColorRGBf color, float power = 1.f)
    {
        DirectionalLight& light = directionalLights.emplace_back();
        light.dir   = direction;
        light.color = color;
        light.power = power;
    }

    static void addPointLight(const Vector3f& pos, const ColorRGBf& color, float power = 1.f)
    {
        PointLight& light = pointLights.emplace_back();
        light.pos   = pos;
        light.color = color;
        light.power = power;
    }

    static void addSpotLight(const Vector3f& pos, const Vector3f& dir, float angleDeg, float outerAngleDeg, const ColorRGBf& color, float power = 1.f)
    {
        SpotLight& light    = spotLights.emplace_back();
        light.pos           = pos;
        light.dir           = dir;
        light.cosAngle      = glm::cos(glm::radians(angleDeg));
        light.outerCosAngle = glm::cos(glm::radians(angleDeg + outerAngleDeg));
        light.color         = color;
        light.power         = power;
    }

    static void setPBRShader(const String& vertsrc, const String& fragsrc)
    {
        GLSLSource vert(vertsrc); GLSLSource frag(fragsrc);
        frag.inject(fmt::format("#define maxDirectionalLights {}", maxDirectionalLights));
        frag.inject(fmt::format("#define maxPointLights       {}", maxPointLights));
        frag.inject(fmt::format("#define maxSpotLights        {}", maxSpotLights));

        frag.inject(fmt::format("#define shadowMapCascadeCount {}", shadowCascadesCount));
        vert.inject(fmt::format("#define shadowMapCascadeCount {}", shadowCascadesCount));

        if (!shader3d.create(vert.string(), frag.string()))
        {
            tlog::error("Failed to compile PBR shader.");
            //tlog::error("Vertex Shader:");
            //tlog::error('\n' + vert.string());
            //tlog::error("Fragment Shader:");
            //tlog::error('\n' + frag.string());
        }
    }

    static void render()
    {
        if (cmds.empty()) { return; }

        Renderer::clearColor(skyColor);
        GL_CHECK(glEnable(GL_DEPTH_TEST));
        GL_CHECK(glEnable(GL_BLEND));
        GL_CHECK(glFrontFace(GL_CCW));

        if (useMSAA) { GL_CHECK(glEnable(GL_MULTISAMPLE)); }
        else         { GL_CHECK(glDisable(GL_MULTISAMPLE)); }

        glEnable(GL_PRIMITIVE_RESTART);
        glPrimitiveRestartIndex(restartIndex);

        auto view = camera.getViewMatrix();
        auto proj = camera.getPerspectiveMatrix();
        shader3d.setMat4f("projection",      proj);
        shader3d.setMat4f("view",            view);
        shader3d.setFloat("ambientStrength", ambientLightStrength);
        shader3d.setVec3f("cameraPos",       camera.pos);
        shader3d.setBool ("fogEnabled",      fog);
        shader3d.setVec3f("fogColor",        Vector3f(fogColor.r, fogColor.g, fogColor.b));
        shader3d.setFloat("viewDistance",    camera.zfar);
        shader3d.setBool ("shadowsEnabled",  shadows);
        shader3d.setInt  ("pcfSteps",        shadowPcfSteps);
        shader3d.setFloat("minShadowBias",   minShadowBias);
        shader3d.setFloat("maxShadowBias",   std::max(minShadowBias, maxShadowBias));

        defaultPrimitiveShader.setMat4f("projection", proj);
        defaultPrimitiveShader.setMat4f("view",       view);

        uploadLights();
        renderShadows();
        renderCSMs();
        renderMeshes();

        // Clear state
        cmds             .clear();
        primitiveVerts   .clear();
        primitiveIndices .clear();
        pointLights      .clear();
        directionalLights.clear();
        spotLights       .clear();
    }

    static void create()
    {
        // Setup Primitive Rendering
        {
            primitiveMesh.setLayout({ Layout::Vec3f(), Layout::Vec4f() });

            defaultPrimitiveShader.create(
                myEmbeds.at("TLib/Embed/Shaders/3d_primitive.vert").asString(),
                myEmbeds.at("TLib/Embed/Shaders/3d_primitive.frag").asString());

            defaultPrimitiveShader.setMat4f("model", glm::mat4(1));
            primitiveVerts.reserve(1024 * 2);
            primitiveIndices.reserve(1024 * 2);
        }

        // Setup PBR Rendering
        {
            setPBRShader(myEmbeds.at("TLib/Embed/Shaders/3d.vert").asString(),
                         myEmbeds.at("TLib/Embed/Shaders/3d.frag").asString());
        }

        // Setup shadows
        initShadows();
        initCSMs();
    }

private:
    static void setCullMode(FaceCullMode mode)
    {
        switch (mode)
        {
            case FaceCullMode::None:
                GL_CHECK(glDisable(GL_CULL_FACE));
                break;
            case FaceCullMode::Front:
                GL_CHECK(glEnable(GL_CULL_FACE));
                glCullFace(GL_FRONT);
                break;
            case FaceCullMode::Back:
                GL_CHECK(glEnable(GL_CULL_FACE));
                glCullFace(GL_BACK);
                break;
            case FaceCullMode::Both:
                GL_CHECK(glEnable(GL_CULL_FACE));
                glCullFace(GL_FRONT_AND_BACK);
                break;
            default: break;
        }
    }

    static void initCSMs()
    {
        csmShader.create(myEmbeds["TLib/Embed/Shaders/csm.vert"]  .asString(),
                         myEmbeds["TLib/Embed/Shaders/empty.frag"].asString());

        csmFbo.create();

        for (uint32_t i = 0; i < shadowCascadesCount; i++)
        {
            auto& tex = csmTextures.emplace_back();
            GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
            tex = makeUnique<Texture>();
            tex->create();
            tex->setData(NULL, shadowSize, shadowSize, TexPixelFormats::DEPTH_COMPONENT, TexInternalFormats::DEPTH_COMPONENT, TexPixelType::Float);
            tex->setFilter(TextureMinFilter::Linear, TextureMagFilter::Linear);
            tex->setUVMode(UVMode::ClampToBorder);
            tex->setBorderColor(ColorRGBAf::white());
            tex->setSwizzle(swizzle); // To make debugging easier on the eyes
            tex->bind();
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.f); // TODO: extend tex class
            //GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
            //GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
        }
    }

    static void renderCSMs()
    {
        if (shadowCascadesCount < 2)
        { return; }

        const float maxDist  = camera.zfar;
        const float minDist  = 20.f;
        const float distDiff = maxDist - minDist;
        const float distStep = distDiff / (shadowCascadesCount-1);

        const uint32_t texStartIndex = 4;

        if (maxDist < minDist) {  }

        for (uint32_t i = 0; i < shadowCascadesCount; i++)
        {
            float zfar = minDist + i * distStep;

            // Get vp
            Mat4f lightSpaceMatrix;
            {
                View3D camCopy   = camera;
                camCopy.zfar     = zfar;

                Vector3f dir     = directionalLights[0].dir;
                auto corners     = getFrustumCornersWorldSpace(camCopy.getPerspectiveMatrix(), camCopy.getViewMatrix());

                Vector3f viewTarget = getFrustumCenter(corners);
                Vector3f viewPos    = (viewTarget - dir);
                Mat4f    lightView  = safeLookAt(viewPos, viewTarget, Vector3f::up(), Vector3f::backward());

                Mat4f lightProjection  = getLightProjection(shadowFrustZMult, corners, lightView).scale(1.f, -1.f, 1.f);
                lightSpaceMatrix = lightProjection * lightView;
            }

            auto& tex = csmTextures[i]; ASSERT(tex);
            csmFbo.setTexture(*tex, FrameBufferAttachmentType::Depth);
            csmFbo.bind();
            Renderer::setViewport(Recti(Vector2i(0), Vector2i(shadowSize, shadowSize)), Vector2f((float)shadowSize));
            csmShader.setMat4f("lightSpaceMatrix", lightSpaceMatrix);

            shader3d.setMat4f(fmt::format("csmlightSpaceMatrices[{}]", i), lightSpaceMatrix);
            shader3d.setFloat(fmt::format("csmEndClipSpace[{}]"      , i), zfar);

            uint32_t texIndex = texStartIndex + i;
            shader3d.setInt(fmt::format("csms[{}]", i), texIndex);
            tex->bind(texIndex);

            //shadowFbo.bind();
            //shader3d.setMat4f("lightSpaceMatrix", lightSpaceMatrix);

            setCullMode(shadowFaceCullMode);
            glClear(GL_DEPTH_BUFFER_BIT);
            for (auto& varCmd : cmds)
            {
                if (is<ModelDrawCmd>(varCmd))
                {
                    ModelDrawCmd& cmd = std::get<ModelDrawCmd>(varCmd);

                    for (auto& mesh : cmd.model->getMeshes())
                    {
                        csmShader.setMat4f("model", cmd.transform.getMatrix());
                        Renderer::draw(csmShader, *mesh.vertices);
                    }
                }
            }

            csmFbo.unbind();
        }
    }

    static void initShadows()
    {
        shadowTex.create();
        shadowTex.setData(NULL, shadowSize, shadowSize, TexPixelFormats::DEPTH_COMPONENT, TexInternalFormats::DEPTH_COMPONENT, TexPixelType::Float);
        shadowTex.setFilter(TextureMinFilter::Linear, TextureMagFilter::Linear);
        shadowTex.setUVMode(UVMode::ClampToBorder);
        shadowTex.setBorderColor(ColorRGBAf::white());
        GLint swizzle[4] = { GL_RED, GL_RED, GL_RED, GL_ONE };
        shadowTex.setSwizzle(swizzle); // To make debugging easier on the eyes
        shadowTex.bind();
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 8.f); // TODO: extend tex class
        //GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
        //GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL));
        shadowFbo.create();
        shadowFbo.setTexture(shadowTex, FrameBufferAttachmentType::Depth);
        shadowShader.create(myEmbeds.at("TLib/Embed/Shaders/light.vert").asString(),
                            myEmbeds.at("TLib/Embed/Shaders/empty.frag").asString());
    }

    static void renderShadows()
    {
        if (!shadows) { return; }

        {
            shader3d.setInt("shadowMap", 3);

            View3D   camCopy = camera;
            Vector3f dir     = directionalLights[0].dir;
            camCopy.zfar     = std::min(shadowDistance, camera.zfar);
            auto corners     = getFrustumCornersWorldSpace(camCopy.getPerspectiveMatrix(), camCopy.getViewMatrix());

            Vector3f viewTarget = getFrustumCenter(corners);
            Vector3f viewPos    = (viewTarget - dir);
            Mat4f    lightView  = safeLookAt(viewPos, viewTarget, Vector3f::up(), Vector3f::backward());

            Mat4f lightProjection  = getLightProjection(shadowFrustZMult, corners, lightView).scale(1.f, -1.f, 1.f);
            Mat4f lightSpaceMatrix = lightProjection * lightView;

            Renderer::setViewport(Recti(Vector2i(0), Vector2i(shadowSize, shadowSize)), Vector2f((float)shadowSize));
            shadowFbo.bind();
            shadowShader.setMat4f("lightSpaceMatrix", lightSpaceMatrix);
            shader3d.setMat4f("lightSpaceMatrix", lightSpaceMatrix);
        }

        // Render Scene
        setCullMode(shadowFaceCullMode);
        {
            glClear(GL_DEPTH_BUFFER_BIT);
            for (auto& varCmd : cmds)
            {
                if (is<ModelDrawCmd>(varCmd))
                {
                    ModelDrawCmd& cmd = std::get<ModelDrawCmd>(varCmd);

                    for (auto& mesh : cmd.model->getMeshes())
                    {
                        shadowShader.setMat4f("model", cmd.transform.getMatrix());
                        Renderer::draw(shadowShader, *mesh.vertices);
                    }
                }
            }

            shadowFbo.unbind();
        }
    }

    static Mat4f getLightProjection(float zmult, const Vector<Vector4f>& corners, const Mat4f& lightView)
    {
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        for (const auto& v : corners)
        {
            const auto trf = lightView * v;
            minX = std::min(minX, trf.x);
            maxX = std::max(maxX, trf.x);
            minY = std::min(minY, trf.y);
            maxY = std::max(maxY, trf.y);
            minZ = std::min(minZ, trf.z);
            maxZ = std::max(maxZ, trf.z);
        }

        if (minZ < 0) { minZ *= shadowFrustZMult; }
        else          { minZ /= shadowFrustZMult; }
        if (maxZ < 0) { maxZ /= shadowFrustZMult; }
        else          { maxZ *= shadowFrustZMult; }

        return glm::ortho(minX, maxX, minY, maxY, minZ, maxZ);
    }

    static Vector3f getFrustumCenter(const Vector<Vector4f>& corners)
    {
        Vector3f center;
        for (const auto& v : corners)
        { center += Vector3f(v); }
        center /= (float)corners.size();
        return center;
    }

    static Mat4f safeLookAt(const Vector3f& lookFrom, const Vector3f& lookTo, Vector3f up, const Vector3f& altUp)
    {
        Vector3f  dir    = lookTo - lookFrom;
        float     dirLen = dir.length();

        // Check if the direction is valid; Also deals with NaN
        if (!(dirLen > 0.0001)) return Mat4f(1); // Just return identity

        dir.normalize();

        if (abs(dir.dot(up)) > .9999f) { up = altUp; }
        return glm::lookAt(lookFrom.toGlm(), lookTo.toGlm(), up.toGlm());
    }

    static void renderMeshes()
    {
        setCullMode(faceCullMode);
        shader3d.setInt ("material.diffuse",   0);
        shader3d.setInt ("material.roughness", 1);
        shader3d.setInt ("material.metallic",  2);

        Renderer::setViewport(Recti(0, 0, Renderer::getFramebufferSize()));

        for (auto& varCmd : cmds)
        {
            if (is<PrimitiveDrawCmd>(varCmd))
            {
                PrimitiveDrawCmd& cmd = std::get<PrimitiveDrawCmd>(varCmd);

                Span<PrimVertex> pos(primitiveVerts.begin()   + cmd.posIndex, cmd.posSize);
                Span<uint32_t>   ind(primitiveIndices.begin() + cmd.indIndex, cmd.indSize);

                primitiveMesh.setData   (pos, AccessType::Dynamic);
                primitiveMesh.setIndices(ind, AccessType::Dynamic);

                RenderState rs; rs.drawMode = cmd.drawMode;
                Renderer::draw(*cmd.shader, primitiveMesh, rs);
            }

            else if (is<ModelDrawCmd>(varCmd))
            {
                ModelDrawCmd& cmd = std::get<ModelDrawCmd>(varCmd);

                for (auto& mesh : cmd.model->getMeshes())
                {
                    mesh.material.textures[(int32_t)TextureType::Diffuse]  ->bind(0);
                    mesh.material.textures[(int32_t)TextureType::Roughness]->bind(1);
                    mesh.material.textures[(int32_t)TextureType::Metalness]->bind(2);
                    shadowTex.bind(3);

                    shader3d.setMat4f("model", cmd.transform.getMatrix());
                    Renderer::draw(shader3d, *mesh.vertices);
                }
            }
        }
    }

    static void uploadLights()
    {
        // Point Lights
        {
            int32_t pointLightCount = std::min<int32_t>(pointLights.size(), maxPointLights);
            shader3d.setInt("pointLightCount", pointLightCount);
            for (int32_t i = 0; i < pointLightCount; i++)
            {
                shader3d.setVec3f(fmt::format("pointLights[{}].localPos",               i), pointLights[i].pos);
                shader3d.setVec3f(fmt::format("pointLights[{}].light.color",            i), pointLights[i].color);
                shader3d.setFloat(fmt::format("pointLights[{}].light.diffuseIntensity", i), pointLights[i].power);
                shader3d.setFloat(fmt::format("pointLights[{}].light.ambientIntensity", i), 1.f);
            }
        }

        // Directional Lights
        {
            int32_t directionalLightCount = std::min<int32_t>(directionalLights.size(), maxDirectionalLights);
            shader3d.setInt("directionalLightCount", directionalLightCount);
            for (int32_t i = 0; i < directionalLightCount; i++)
            {
                shader3d.setVec3f(fmt::format("directionalLights[{}].dir",                    i), directionalLights[i].dir);
                shader3d.setVec3f(fmt::format("directionalLights[{}].light.color",            i), directionalLights[i].color);
                shader3d.setFloat(fmt::format("directionalLights[{}].light.diffuseIntensity", i), directionalLights[i].power);
                shader3d.setFloat(fmt::format("directionalLights[{}].light.ambientIntensity", i), 1.f);
            }
        }

        // Spot Lights
        {
            int32_t spotLightCount = std::min<int32_t>(spotLights.size(), maxSpotLights);
            shader3d.setInt("spotLightCount", spotLightCount);
            for (int32_t i = 0; i < spotLightCount; i++)
            {
                shader3d.setVec3f(fmt::format("spotLights[{}].pos",                    i), spotLights[i].pos);
                shader3d.setVec3f(fmt::format("spotLights[{}].dir",                    i), spotLights[i].dir);
                shader3d.setFloat(fmt::format("spotLights[{}].cosAngle",               i), spotLights[i].cosAngle);
                shader3d.setFloat(fmt::format("spotLights[{}].outerCosAngle",          i), spotLights[i].outerCosAngle);
                shader3d.setVec3f(fmt::format("spotLights[{}].light.color",            i), spotLights[i].color);
                shader3d.setFloat(fmt::format("spotLights[{}].light.diffuseIntensity", i), spotLights[i].power);
                shader3d.setFloat(fmt::format("spotLights[{}].light.ambientIntensity", i), 1.f);
            }
        }
    }
};