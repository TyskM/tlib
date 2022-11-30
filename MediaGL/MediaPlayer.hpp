#pragma once

// TODO: Test multiple media players
// Note: Running libvlc functions like libvlc_new with
// the visual studio debugger attached makes the program hang
// even when the functions are run in a different thread.
// frick microsoft and frick libvlc

// https://github.com/videolan/libvlcpp/blob/master/examples/imem/imem.cpp

#include "Renderer.hpp"
#include "../Misc.hpp"

#include <mutex>
#include <atomic>
#include <string>
#include <chrono>
using namespace std::chrono;

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

#include <thirdparty/vlc4/vlc.h>
#include <thirdparty/vlc4/libvlc_events.h>

#pragma comment(lib, "Crypt32.lib")
#include <MediaInfo/MediaInfo.h>


static constexpr char const* libvlcargs[] =
{
    //"--no-plugins-scan"
    //"--verbose=2"
    "--no-xlib",
    "--no-video-title-show",
    "--no-lua",
    "--reset-plugins-cache"
};

enum class MediaType
{
    Invalid,
    Image,
    Video,
    Audio,
    COUNT
};

struct MediaData
{
    bool success = false;

    MediaType mediaType = MediaType::Invalid;
    int width;
    int height;
};

// Time in milliseconds
using TimeMS = int64_t;

struct MediaPlayer : NonCopyable
{
    // Globals
    static inline libvlc_instance_t* vlc = nullptr;

    // Read-only values
    int width  = -1;
    int height = -1;
    int pitch  = -1;
    bool loop = true;
    MediaType mediaType = MediaType::Invalid;
    std::string mediaPath;
    Texture texture;

    bool needUpdate = false;
    std::vector<Uint8> pixelBuffer;

    libvlc_media_player_t* mplayer = nullptr;
    libvlc_event_manager_t* mplayerEvMan = nullptr;
    libvlc_media_t* media = nullptr;

    std::mutex mutex;
    std::unique_ptr<std::thread> vlcThread;

    MediaPlayer() { }

    MediaPlayer(const std::string& path)
    {
        loadFromPath(path);
    }

    ~MediaPlayer()
    {
        reset();
    }

    inline Vector2i getSize() const noexcept
    {
        return { width, height };
    }

    bool created() { return texture.created(); }

    void reset()
    {
        if (vlcThread) { vlcThread->join(); vlcThread.reset(); }
        if (media) { libvlc_media_release(media); media = nullptr; }
        if (mplayer) { libvlc_media_player_release(mplayer); mplayer = nullptr; }

        texture.reset();
    }


    void create()
    {
        reset();

        if (vlc == nullptr)
            vlc = libvlc_new(sizeof(libvlcargs) / sizeof(*libvlcargs), libvlcargs);
        ASSERTMSG(vlc, "Failed to init VLC");

        if (!mplayerEvMan) mplayerEvMan = libvlc_media_player_event_manager(mplayer);
        //libvlc_event_attach( mplayerEvMan, libvlc_MediaPlayerEndReached, vlcEvent, this );
    }

    bool loadFromPath(const std::string& path)
    {
        create();

        mediaPath = path;
        
        MediaData md = tryGetMediaData(path);
        if (md.success == false)
        {
            std::cout << "Failed to read file data at path: " << path << std::endl;
            reset(); return false;
        }

        width = md.width;
        height = md.height;

        pitch = 4 * width;
        const auto bufferSize = (height * pitch + 32);
        pixelBuffer.resize(bufferSize);

        texture.create();
        texture.setData(0, width, height, TextureFiltering::Linear);

        // Swizzle because incoming format is BGRA
        texture.bind();
        GLint swizzleMask[] = {GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
        texture.unbind();

        media = libvlc_media_new_path(mediaPath.c_str());
        ASSERTMSG(media, "Failed to create media");

        mplayer = libvlc_media_player_new_from_media (vlc, media);
        ASSERTMSG(mplayer, "Failed to create media player");

        libvlc_video_set_format(mplayer, "RV32", width, height, pitch);
        libvlc_video_set_callbacks(mplayer, lock, unlock, nullptr, this);
        restart();
        play();

        return true;
    }

    bool isPlaying()
    { return libvlc_media_player_is_playing(mplayer); }

    int play()
    { return libvlc_media_player_play(mplayer); }

    void pause()
    { if (isPlaying()) libvlc_media_player_pause(mplayer); }

    void togglePlayPause()
    { if (isPlaying()) { pause(); } else { play(); } }

    void stop()
    { libvlc_media_player_stop_async(mplayer); }

    void setPositionPercent(const double pos, bool fastSeek = true)
    { libvlc_media_player_set_position(mplayer, pos, fastSeek); }

    double getPositionPercent()
    { return libvlc_media_player_get_position(mplayer); }

    void setPosition(TimeMS time, bool fastSeek = true)
    { if (isSeekable()) libvlc_media_player_set_time(mplayer, time, fastSeek); }

    TimeMS getPosition()
    { return libvlc_media_player_get_time(mplayer); }

    TimeMS getMediaDuration()
    { return libvlc_media_player_get_length(mplayer); }

    libvlc_media_player_role getRole()
    { return (libvlc_media_player_role)libvlc_media_player_get_role(mplayer); }

    libvlc_state_t getState()
    { return libvlc_media_player_get_state(mplayer); }

    void restart()
    { setPosition(0, false); play(); }

    bool isSeekable()
    { return libvlc_media_player_is_seekable(mplayer); }

    void addOption(const char* options)
    {
        ASSERT(media);
        libvlc_media_add_option(media, options);
    }

    // If you're not using the draw() method, and instead using
    // the texture member directly, call this before using it.
    // This method is already called in the draw() method.
    bool tryUpdateTexture()
    {
        if (needUpdate && mutex.try_lock())
        {
            if (loop && getMediaDuration() == 0) { restart(); }

            texture.setData(pixelBuffer.data(), width, height);
            needUpdate = false;
            mutex.unlock();
            return true;
        }
    }

    void draw(Renderer& renderer, const Rectf& rect, float delta)
    {
        if (!created()) return;

        tryUpdateTexture();
        renderer.drawTexture(texture, rect);
    }

    static inline MediaData tryGetMediaData(const std::string& path)
    {
        using namespace MediaInfoLib;

        MediaData md;

        // There's probably a way to do this with VLC but trash api :))))))
        MediaInfo mi;
        if (!mi.Open(String(std::wstring(path.begin(), path.end()))))
        { md.success = false; return md; }

        //#ifdef _DEBUG
        //std::wcout << mi.Inform() << std::endl;
        //#endif

        int audioStreams = std::stol(mi.Get(Stream_General, 0, L"AudioCount"));
        int videoStreams = std::stol(mi.Get(Stream_General, 0, L"VideoCount"));
        int imageStreams = std::stol(mi.Get(Stream_General, 0, L"ImageCount"));

        // HACK: Probably forgetting something here
        if      (videoStreams > 0) { md.mediaType = MediaType::Video; }
        else if (audioStreams > 0) { md.mediaType = MediaType::Audio; }
        else if (imageStreams > 0) { md.mediaType = MediaType::Image; }
        else { md.mediaType = MediaType::Invalid; }

        std::array<stream_t, 2> streamsToCheck = {Stream_Video, Stream_Image};
        for (auto& streamType : streamsToCheck)
        {
            try
            {
                auto wstr = mi.Get(streamType, 0, L"Width");
                auto hstr = mi.Get(streamType, 0, L"Height");
                md.width = std::stol(wstr);
                md.height = std::stol(hstr);
                ASSERT( md.width > 0 && md.height > 0 );
                break;
            }
            catch (std::invalid_argument e)
            { md.width = 0; md.height = 0; }
        }

        md.success = true;
        return md;
    }

    static void vlcEvent(const libvlc_event_t* event, void* data)
    {

    }

    static void* lock(void* data, void** planes)
    {
        MediaPlayer& c = *(MediaPlayer*)data;
        c.mutex.lock();
        planes[0] = (void *)c.pixelBuffer.data();
        return NULL; // Picture identifier, not needed here.
    }

    static void unlock(void* data, void* picture, void* const* planes)
    {
        MediaPlayer& c = *(MediaPlayer*)data;

        c.needUpdate = true;

        c.mutex.unlock();
    }
};
