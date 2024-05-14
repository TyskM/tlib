#pragma once

#define NOMINMAX
#include <TLib/Media/GL/GLHelpers.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <unordered_map>
#include <TLib/DataStructures.hpp>
#include <TLib/NonAssignable.hpp>
#include <TLib/Macros.hpp>
#include <TLib/Media/GL/GLState.hpp>
#include <TLib/Media/GL/UniformBuffer.hpp>
#include <TLib/Containers/Array.hpp>
#include <TLib/Containers/UnorderedMap.hpp>
#include <TLib/Types/ColorRGBf.hpp>

class Shader : NonCopyable
{
    GLuint glHandle = 0;
    mutable std::unordered_map<String, int> _uniformCache;

    void cleanup(Vector<GLuint>& shaders, GLuint program)
    {
        for (auto& shader : shaders)
        {
            if (shader) { glDeleteShader(shader); }
        }
        if (program)
        { glDeleteProgram(program); }
        GL_CHECK_NOABORT((void)0);
    }

    bool setup(const char* vertData, const char* fragData, const char* geomData = nullptr)
    {
        Vector<GLuint> shaders;
        GLuint tmpHandle = glCreateProgram();

        {
            GLuint& vertShader = shaders.emplace_back();
            vertShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource (vertShader, 1, &vertData, NULL);
            glCompileShader(vertShader);
            if (!verifyShaderCompilation(vertShader))
            { cleanup(shaders, tmpHandle); return false; }
            glAttachShader(tmpHandle, vertShader);
        }

        {
            GLuint& fragShader = shaders.emplace_back();
            fragShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource (fragShader, 1, &fragData, NULL);
            glCompileShader(fragShader);
            if (!verifyShaderCompilation(fragShader))
            { cleanup(shaders, tmpHandle); return false; }
            glAttachShader(tmpHandle, fragShader);
        }

        if (geomData)
        {
            GLuint& geomShader = shaders.emplace_back();
            geomShader = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geomShader, 1, &geomData, NULL);
            glCompileShader(geomShader);
            if (!verifyShaderCompilation(geomShader))
            { cleanup(shaders, tmpHandle); return false; }
            glAttachShader(tmpHandle, geomShader);
        }

        glLinkProgram (tmpHandle);
        if (!verifyProgamLinkage(tmpHandle))
        { cleanup(shaders, tmpHandle); return false; }

        cleanup(shaders, 0);
        reset();
        glHandle = tmpHandle;

        return true;
    }

    void showFailedToFindUniformError(const String& uniName) const
    {
        tlog::warn(R"(
The uniform '{}' could not be found.
It could be missing, misspelled,
or the GLSL compiler could have optimized it away.
(If it's unused in the shader code)
)", uniName);
    }

public:
    Shader() { }
    Shader(const char* vertData, const char* fragData) { create(vertData, fragData); }
    Shader(Shader&& other) noexcept { operator=(std::move(other)); }
    Shader& operator=(Shader&& other) noexcept
    {
        reset();
        glHandle       = other.glHandle;
        other.glHandle = NULL;
        // Do NOT copy the uniform map
        return *this;
    }

    ~Shader() { reset(); }

    void reset()
    {
        if (created())
        {
            glDeleteProgram(glHandle);
            _uniformCache.clear();
        }
    }

    bool create(const char* vertData, const char* fragData, const char* geomData = nullptr)
    {
        return setup(vertData, fragData);
    }

    bool create(const String& vert, const String& frag)
    { return create(vert.c_str(), frag.c_str()); }

    bool create(const String& vert, const String& frag, const String& geom)
    { return create(vert.c_str(), frag.c_str(), geom.c_str()); }

    bool created() const
    { return glHandle != 0; }

    void bind()
    {
        ASSERTMSG(created(), "Forgot to call shader::create()");
        //if (glState.boundShader == this) { return; }
        GL_CHECK(glUseProgram(glHandle));
        glState.boundShader = this;
    }

    static inline void unbind()
    {
        glUseProgram(0);
        glState.boundShader = nullptr;
    }

    void setBool(const String& name, bool value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform1i(loc, static_cast<int>(value)));
    }

    void setInt(const String& name, int value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform1i(loc, value));
    }

    GLint getInt(const String& name)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return {}; }
        GLint ret;
        GL_CHECK(glGetUniformiv(glHandle, loc, &ret));
        return ret;
    }

    GLfloat getFloat(const String& name)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return {}; }
        GLfloat ret;
        GL_CHECK(glGetUniformfv(glHandle, loc, &ret));
        return ret;
    }

    glm::vec2 getVec2f(const String& name)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return {}; }
        Array<GLfloat, 2> ret{};
        GL_CHECK(glGetnUniformfv(glHandle, loc, sizeof(GLfloat) * 2, ret.data()));
        return { ret[0], ret[1] };
    }

    glm::vec3 getVec3f(const String& name)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return {}; }
        Array<GLfloat, 3> ret{ 0,0,0 };
        GL_CHECK(glGetnUniformfv(glHandle, loc, sizeof(GLfloat) * 3, ret.data()));
        return {ret[0], ret[1], ret[2]};
    }

    glm::vec4 getVec4f(const String& name)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return {}; }
        Array<GLfloat, 4> ret{ 0,0,0,0 };
        GL_CHECK(glGetnUniformfv(glHandle, loc, sizeof(GLfloat) * 4, ret.data()));
        return { ret[0], ret[1], ret[2], ret[3] };
    }

    void setFloat(const String& name, float value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform1f(loc, value));
    }
    
    void setVec2f(const String& name, float x, float y)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform2f(loc, x, y));
    }

    void setVec2f(const String& name, glm::vec2 value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform2f(loc, value.x, value.y));
    }

    void setVec3f(const String& name, float x, float y, float z)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform3f(loc, x, y, z));
    }

    void setVec3f(const String& name, glm::vec3 value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform3f(loc, value.x, value.y, value.z));
    }

    void setVec3f(const String& name, const Vector3f& value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform3f(loc, value.x, value.y, value.z));
    }

    void setVec4f(const String& name, float x, float y, float z, float w)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform4f(loc, x, y, z, w));
    }

    void setVec4f(const String& name, const Vector4<float> value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform4f(loc, value.x, value.y, value.z, value.w));
    }

    void setVec4f(const String& name, glm::vec4 value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniform4f(loc, value.x, value.y, value.z, value.w));
    }

    void setMat4f(const String& name, glm::mat4 value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniformMatrix4fv(loc, 1, false, glm::value_ptr(value)));
    }

    void setMat4f(const String& name, const Mat4f& value)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        GL_CHECK(glUniformMatrix4fv(loc, 1, false, value.data()));
    }

    void setUniformBlock(const String& name, UniformBuffer& ubo, int index)
    {
        ubo.setBufferBase(index);
        int uniformIndex = glGetUniformBlockIndex(glHandle, name.c_str());
        glUniformBlockBinding(glHandle, uniformIndex, index);
    }

    template <typename ContainerType>
    void setMat4fArray(const String& name, const ContainerType& value, size_t count)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        ASSERT(value.size() > 0);
        GL_CHECK(glUniformMatrix4fv(loc, count, false, glm::value_ptr(value[0])));
    }

    template <typename ContainerType>
    void setVec2fArray(const String& name, const ContainerType& value, size_t count)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        ASSERT(value.size() > 0);
        GL_CHECK(glUniform2fv(loc, count, glm::value_ptr(value[0])));
    }

    template <typename ContainerType>
    void setVec4fArray(const String& name, const ContainerType& value, size_t count)
    {
        bind();
        auto loc = getUniformLocation(name); if (loc < 0) { return; }
        ASSERT(value.size() > 0);
        GL_CHECK(glUniform4fv(loc, count, glm::value_ptr(value[0])));
    }

    GLint getUniformLocation(const String& name) const
    {
        const auto& it = _uniformCache.find(name);
        if (it != _uniformCache.end())
        { return it->second; }

        GLint loc = glGetUniformLocation(glHandle, name.c_str());
        if (loc < 0) { showFailedToFindUniformError(name); }

        checkOpenGLError("glGetUniformLocation", __FILE__, __LINE__);
        _uniformCache.insert_or_assign(name, loc);
        return loc;
    }

    static inline bool verifyShaderCompilation(GLuint shaderHandle)
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
            tlog::critical("Failed to compile shader. Info:\n{}", infoLog);
            return false;
        };
        return true;
    }

    static inline bool verifyProgamLinkage(GLuint programHandle)
    {
        int success;
        char infoLog[512];
        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(programHandle, 512, NULL, infoLog);
            tlog::critical("Failed to link shader program. Info:\n{}", infoLog);
            return false;
        }
        return true;
    }
};
