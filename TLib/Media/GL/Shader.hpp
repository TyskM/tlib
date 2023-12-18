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

class Shader : NonCopyable
{
    GLuint glHandle = 0;
    mutable std::unordered_map<String, GLint> _uniformCache;

public:
    Shader() { }
    Shader(const char* vertData, const char* fragData) { create(vertData, fragData); }
    Shader(Shader&& other) noexcept { operator=(std::move(other)); }
    Shader& operator=(Shader&& other) noexcept
    {
        glHandle = other.glHandle;
        other.glHandle = NULL;
        _uniformCache = other._uniformCache;
        return *this;
    }

    ~Shader()
    {
        if (created())
        { glDeleteProgram(glHandle); }
    }

    void create(const char* vertData, const char* fragData)
    {
        GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertShader, 1, &vertData, NULL);
        glCompileShader(vertShader);
        verifyShaderCompilation(vertShader);

        GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragShader, 1, &fragData, NULL);
        glCompileShader(fragShader);
        verifyShaderCompilation(fragShader);

        glHandle = glCreateProgram();
        glAttachShader(glHandle, vertShader);
        glAttachShader(glHandle, fragShader);
        glLinkProgram(glHandle);
        verifyProgamLinkage(glHandle);

        glDeleteShader(vertShader);
        glDeleteShader(fragShader);
    }

    void create(const String& vert, const String& frag)
    { create(vert.c_str(), frag.c_str()); }

    bool created() const
    { return glHandle != 0; }

    void bind()
    {
        ASSERTMSG(created(), "Forgot to call shader::create()");
        if (glState.boundShader == this) { return; }
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
        GL_CHECK(glUniform1i(getUniformLocation(name.c_str()), static_cast<int>(value)));
    }

    void setInt(const String& name, int value)
    {
        bind();
        GL_CHECK(glUniform1i(getUniformLocation(name.c_str()), value));
    }

    GLint getInt(const String& name)
    {
        bind();
        GLint ret;
        GL_CHECK(glGetUniformiv(glHandle, getUniformLocation(name.c_str()), &ret));
        return ret;
    }

    void setFloat(const String& name, float value)
    {
        bind();
        GL_CHECK(glUniform1f(getUniformLocation(name.c_str()), value));
    }
    
    void setVec2f(const String& name, float x, float y)
    {
        bind();
        GL_CHECK(glUniform2f(getUniformLocation(name.c_str()), x, y));
    }

    void setVec2f(const String& name, glm::vec2 value)
    {
        bind();
        GL_CHECK(glUniform2f(getUniformLocation(name.c_str()), value.x, value.y));
    }

    void setVec3f(const String& name, float x, float y, float z)
    {
        bind();
        GL_CHECK(glUniform3f(getUniformLocation(name.c_str()), x, y, z));
    }

    void setVec3f(const String& name, glm::vec3 value)
    {
        bind();
        GL_CHECK(glUniform3f(getUniformLocation(name.c_str()), value.x, value.y, value.z));
    }

    void setVec4f(const String& name, float x, float y, float z, float w)
    {
        bind();
        GL_CHECK(glUniform4f(getUniformLocation(name.c_str()), x, y, z, w));
    }

    void setVec4f(const String& name, glm::vec4 value)
    {
        bind();
        GL_CHECK(glUniform4f(getUniformLocation(name.c_str()), value.x, value.y, value.z, value.w));
    }

    void setMat4f(const String& name, glm::mat4 value)
    {
        bind();
        GL_CHECK(glUniformMatrix4fv(getUniformLocation(name.c_str()), 1, false, glm::value_ptr(value)));
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
        ASSERT(value.size() > 0);
        GL_CHECK(glUniformMatrix4fv(getUniformLocation(name.c_str()), count, false, glm::value_ptr(value[0])));
    }

    template <typename ContainerType>
    void setVec2fArray(const String& name, const ContainerType& value, size_t count)
    {
        bind();
        ASSERT(value.size() > 0);
        GL_CHECK(glUniform2fv(getUniformLocation(name.c_str()), count, glm::value_ptr(value[0])));
    }

    template <typename ContainerType>
    void setVec4fArray(const String& name, const ContainerType& value, size_t count)
    {
        bind();
        ASSERT(value.size() > 0);
        GL_CHECK(glUniform4fv(getUniformLocation(name.c_str()), count, glm::value_ptr(value[0])));
    }

    bool created()
    { return glHandle != 0; }

    GLint getUniformLocation(const String& name) const
    {
        auto it = _uniformCache.find(name);
        if (it != _uniformCache.end())
        { return it->second; }

        GLint loc = glGetUniformLocation(glHandle, name.c_str());
        _uniformCache[name] = loc;
        return loc;
    }

    static inline void verifyShaderCompilation(GLuint shaderHandle)
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
            tlog::critical("Failed to compile shader. Info:\n{}", infoLog);
        };
    }

    static inline void verifyProgamLinkage(GLuint programHandle)
    {
        int success;
        char infoLog[512];
        glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(programHandle, 512, NULL, infoLog);
            tlog::critical("Failed to link shader program. Info:\n{}", infoLog);
        }
    }
};
