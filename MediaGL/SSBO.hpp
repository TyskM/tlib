
#include "GLHelpers.hpp"
#include "../NonAssignable.hpp"

// Call bind before calling members other than create() or unbind()
struct SSBO : NonAssignable
{
    GLuint glHandle = 0;

    SSBO() = default;
    ~SSBO() { reset(); }

    void create()
    {
        glGenBuffers(1, &glHandle);
        bind();
        bufferData(nullptr);
        setBufferBase(0);
        unbind();
    }

    bool created() { return glHandle != 0; }

    void reset()
    {
        if (created())
        {
            glDeleteBuffers(1, &glHandle);
            glHandle = 0;
        }
    }

    template <typename T>
    void bufferData(const T& dataObj = nullptr, int storageType = GL_DYNAMIC_DRAW)
    {
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T), &dataObj, storageType);
    }

    // This guy explain it good
    // https://stackoverflow.com/questions/54955186/difference-between-glbindbuffer-and-glbindbufferbase
    void setBufferBase(int index = 0)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, glHandle);
    }

    void bind()
    {
        if (glState.boundSSBO == this) { return; }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, glHandle);
        glState.boundSSBO = this;
    }

    static inline void unbind()
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        glState.boundSSBO = nullptr;
    }

    bool bound() { return glState.boundSSBO == this; }
};