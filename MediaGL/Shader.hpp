#pragma once

#define NOMINMAX
#include "GLHelpers.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include "../DataStructures.hpp"
#include "../NonAssignable.hpp"
#include "../Macros.hpp"
#include "GLState.hpp"

struct Shader : NonAssignable
{
    GLuint glHandle = 0;

    Shader() { }
    Shader(const char* vertData, const char* fragData) { create(vertData, fragData); }

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

    void setBool(const std::string& name, bool value)
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform1i(glGetUniformLocation(glHandle, name.c_str()), static_cast<int>(value)));
    }

    void setInt(const std::string& name, int value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform1i(glGetUniformLocation(glHandle, name.c_str()), value));
    }

    void setFloat(const std::string& name, float value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform1f(glGetUniformLocation(glHandle, name.c_str()), value));
    }
    
    void setVec2f(const std::string& name, float x, float y) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform2f(glGetUniformLocation(glHandle, name.c_str()), x, y));
    }

    void setVec2f(const std::string& name, glm::vec2 value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform2f(glGetUniformLocation(glHandle, name.c_str()), value.x, value.y));
    }

    void setVec3f(const std::string& name, float x, float y, float z) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform3f(glGetUniformLocation(glHandle, name.c_str()), x, y, z));
    }

    void setVec3f(const std::string& name, glm::vec3 value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform3f(glGetUniformLocation(glHandle, name.c_str()), value.x, value.y, value.z));
    }

    void setVec4f(const std::string& name, float x, float y, float z, float w) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform4f(glGetUniformLocation(glHandle, name.c_str()), x, y, z, w));
    }

    void setVec4f(const std::string& name, glm::vec4 value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniform4f(glGetUniformLocation(glHandle, name.c_str()), value.x, value.y, value.z, value.w));
    }

    void setMat4f(const std::string& name, glm::mat4 value) const
    {
        ASSERTMSG(glState.boundShader == this, "Call bind before setting uniforms");
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(glHandle, name.c_str()), 1, false, glm::value_ptr(value)));
    }

    bool created()
    { return glHandle != 0; }

    static inline void verifyShaderCompilation(GLuint shaderHandle)
    {
        int success;
        char infoLog[512];
        glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetShaderInfoLog(shaderHandle, 512, NULL, infoLog);
            std::cerr << "!!! ERROR: Failed to compile shader. Info:\n" << infoLog << std::endl;
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
            std::cout << "!!! ERROR: Failed to link shader program. Info:\n" << infoLog << std::endl;
        }
    }
};
