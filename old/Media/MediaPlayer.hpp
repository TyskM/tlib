#pragma once

// https://github.com/videolan/libvlcpp/blob/master/examples/imem/imem.cpp

#include "Renderer.hpp"

#include <mutex>
#include <atomic>
#include <string>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Dnsapi.lib")
#pragma comment(lib, "dxva2.lib")
#pragma comment(lib, "evr.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfplay.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "libvlccore.lib")
#pragma comment(lib, "libvlc.lib")
//#define ssize_t SSIZE_T
#include <thirdparty/vlc4/vlc.h>
#include <thirdparty/vlcpp/vlc.hpp>

#pragma comment(lib, "Crypt32.lib")
#include <MediaInfo/MediaInfo.h>

static constexpr SDL_PixelFormatEnum format = SDL_PIXELFORMAT_BGR565;
static constexpr const char* vlcFormat = "RV16";

static constexpr char const* libvlcargs[] =
{
    //"--no-audio", /* skip any audio track */
    "--no-xlib", /* tell VLC to not use Xlib */
    "--verbose=2"
    "--no-video-title-show",
    "--reset-plugins-cache"
    // /\ TODO: Check if plugins.dat exists in plugin folder, then pass this flag if it doesn't
};

static int SDL_CalculatePitch(Uint32 format, int width)
{
    int pitch;

    if (SDL_ISPIXELFORMAT_FOURCC(format) || SDL_BITSPERPIXEL(format) >= 8)
    { pitch = (width * SDL_BYTESPERPIXEL(format)); }
    else { pitch = ((width * SDL_BITSPERPIXEL(format)) + 7) / 8; }
    pitch = (pitch + 3) & ~3;   /* 4-byte aligning for speed */
    return pitch;
}

struct MediaPlayer : NonCopyable
{
    static void* lock(void** planes)
    {
        //MediaPlayer& c = *(MediaPlayer*)data;
        c.mutex.lock();
        planes[0] = (void *)c.pixelBuffer.data();
        return NULL; // Picture identifier, not needed here.
    }

    static void unlock(void* picture, void* const* planes)
    {
        //MediaPlayer& c = *(MediaPlayer*)data;

        c.needUpdate = true;

        c.mutex.unlock();
    }

    std::mutex mutex;
    Texture texture;
    Renderer* renderer = nullptr;
    bool needUpdate = false;
    std::vector<Uint8> pixelBuffer;
    int width;
    int height;
    int pitch;

    VLC::Instance vlc;
    VLC::MediaPlayer mp;

    MediaPlayer() { }

    MediaPlayer(Renderer& renderer, const std::string& path)
    {
        loadFromPath(renderer, path);
    }

    ~MediaPlayer()
    {
        reset();
    }

    inline Vector2i getSize() const noexcept
    {
        return { width, height };
    }

    bool isCreated() { return texture.isCreated(); }

    void reset()
    {
        texture.destroy();
        renderer = nullptr;
    }

    void loadFromPath(Renderer& renderer, const std::string& path)
    {
        reset();

        this->renderer = &renderer;
        
        MediaInfoLib::MediaInfo mi;
        if (!mi.Open(MediaInfoLib::String(std::wstring(path.begin(), path.end()))))
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            reset(); return;
        }

        auto w = mi.Get(MediaInfoLib::Stream_Video, 0, L"Width");
        auto h = mi.Get(MediaInfoLib::Stream_Video, 0, L"Height");
        width = std::stol(w);
        height = std::stol(h);
        ASSERT( width > 0 && height > 0 );

        pitch = SDL_CalculatePitch(format, width);

        const auto bufferSize = ( height * pitch ) + 32;
        pixelBuffer.resize(bufferSize);

        texture.create(renderer, format, SDL_TEXTUREACCESS_STREAMING, width, height);

        vlc = VLC::Instance(sizeof(libvlcargs) / sizeof(*libvlcargs), libvlcargs);
        mp = VLC::MediaPlayer(vlc);
        mp.setVideoFormat(vlcFormat, width, height, pitch);
        mp.setVideoCallbacks(lock, unlock, nullptr);
        VLC::Media media(path, VLC::Media::FromPath);
        mp = VLC::MediaPlayer(vlc, media);
        mp.play();
        mp.stopAsync();
    }

    void restart()
    {
        std::cout << "!!!!! Restart\n";
    }

    void draw(const SDL_Rect& rect, float delta)
    {
        if (!isCreated()) return;

        if (needUpdate && mutex.try_lock())
        {
            SDL_UpdateTexture(texture, NULL, pixelBuffer.data(), pitch);
            needUpdate = false;
            mutex.unlock();
        }

        renderer->drawTexture(texture, rect);
    }
};
