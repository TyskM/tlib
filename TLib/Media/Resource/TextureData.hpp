
// CPU Texture information
struct TextureData : NonCopyable
{
    stbi_uc* ptr          = nullptr;
    int      width        = 0;
    int      height       = 0;
    int      channelCount = 0;

    void loadFromPath(const String& path, int reqComp = 4)
    {
        reset();
        ptr = stbi_load(path.c_str(), &width, &height, &channelCount, reqComp);
    }

    void reset()
    { stbi_image_free(ptr); }

    bool valid() { return ptr != nullptr; }

    TextureData() = default;

    // @param reqComp: the required number of channels. ex: 4 for RGBA
    TextureData(const String& path, int reqComp = 4)
    { loadFromPath(path, reqComp); }

    TextureData(stbi_uc* data, int width, int height, int channelCount) :
        ptr{ data }, width{ width }, height{ height }, channelCount{ channelCount } { }

    ~TextureData() noexcept { reset(); }

    TextureData(TextureData&& other) noexcept
    {
        ptr = other.ptr;
        other.ptr = nullptr;
    };

    operator bool() { valid(); }
};
