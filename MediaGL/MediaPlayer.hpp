#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _DISABLE_VECTOR_ANNOTATION
#pragma comment(lib, "Secur32.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Mfplat.lib")
#pragma comment(lib, "Mfuuid.lib")
#pragma comment(lib, "Cfgmgr32.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Vfw32.lib")
#pragma comment(lib, "Shlwapi.lib")

#include "../Logging.hpp"
#include "../Misc.hpp"
#include "../Threading.hpp"

#include <vector>
#include <queue>
#include <deque>

extern "C"
{
    #include <libavutil/time.h>
}
#include <magic_enum.hpp>
#include <avcpp/av.h>
#include <avcpp/ffmpeg.h>
#include <avcpp/stream.h>
#include <avcpp/frame.h>
#include <avcpp/packet.h>
#include <avcpp/avtime.h>
#include <avcpp/codec.h>
#include <avcpp/format.h>
#include <avcpp/formatcontext.h>
#include <avcpp/codeccontext.h>
#include <avcpp/audioresampler.h>
#include <avcpp/videorescaler.h>
#include <avcpp/averror.h>
using std::error_code;

// TODO: optimize!!
// TODO: Decode 1 video frame after seek, to allow previewing frames without playing
struct MediaPlayer : NonAssignable
{
    enum Code : int
    {
        Success      = 1,
        DecodedVideo = 2,
        DecodedAudio = 3,
        EndOfFile    = 4,
        EndOfVideoFile = 5,
        EndOfImageFile = 7,

        Failure            = -1,
        ErrorReadingPacket = -2,
        ErrorDecodingVideo = -3,
        ErrorDecodingAudio = -4
    };


    static constexpr bool audioEnabled = true;

    void loadFromFile(const std::string& path)
    {
        reset();
        av::setFFmpegLoggingLevel(AV_LOG_ERROR);

        error_code ec;
        demuxer.openInput(path, ec);
        if (ec) { tlog::error("Failed to create demuxer with file: '{}'\nReason: {}", path, ec.message()); return; }

        demuxer.findStreamInfo(ec);
        if (ec) { tlog::error("Can't find stream info: {}", ec.message()); return; }

        for (size_t id = 0; id < demuxer.streamsCount(); ++id)
        {
            auto stream = demuxer.stream(id);
            if (stream.mediaType() == AVMEDIA_TYPE_VIDEO)
            {
                videoStreamID = id;
                videoStream = stream;
            }
            else if (audioEnabled && stream.mediaType() == AVMEDIA_TYPE_AUDIO)
            {
                audioStreamID = id;
                audioStream = stream;
            }
        }

        if (videoStreamID == -1 && audioStreamID == -1)
        { tlog::error("Failed to find video AND audio stream! AAAAAHHHHHHHHHH"); return; }

        if (videoStream.isValid())
        {
            videoDecoder = av::VideoDecoderContext(videoStream);
            videoDecoder.setCodec(av::findDecodingCodec(videoDecoder.raw()->codec_id));
            videoDecoder.setRefCountedFrames(false); // Ref counting is leaky, unnecessary and cringe
            videoDecoder.open(ec);
            if (ec)
            { tlog::error("Error creating video decoder"); goto afterVideoInit; }

            imgConverter = av::VideoRescaler(videoDecoder.width(), videoDecoder.height(), AV_PIX_FMT_RGB24, SWS_BICUBIC);
            // imgConverter is invalid even after created. it work weird, look:
            // https://github.com/h4tr3d/avcpp/blob/master/src/videorescaler.cpp
        }
    afterVideoInit:

        if (audioStream.isValid())
        {
            audioDecoder = av::AudioDecoderContext(audioStream);
            audioDecoder.setCodec(av::findDecodingCodec(audioDecoder.raw()->codec_id));
            audioDecoder.setRefCountedFrames(false);
            audioDecoder.open(ec);
            if (ec)
            { tlog::error("Error creating audio decoder"); goto afterAudioInit; }

            audioDevice.specWant.freq     = audioDecoder.sampleRate();
            audioDevice.specWant.format   = AUDIO_S16SYS;
            audioDevice.specWant.silence  = 0;
            audioDevice.specWant.channels = audioDecoder.channels();
            audioDevice.specWant.samples  = 1024;
            audioDevice.specWant.userdata = this;

            if (!audioDevice.create())
            {
                tlog::error("Error creating SDL audio device. Error: {}", SDL_GetError());
                goto afterAudioInit; // TODO: Reset audio func
            }

            resampler.init(audioDecoder.channelLayout(), audioDecoder.sampleRate(), AV_SAMPLE_FMT_S16,
                           audioDecoder.channelLayout(), audioDecoder.sampleRate(), audioDecoder.sampleFormat(), ec);
            if (ec || !resampler.isValid())
            {
                tlog::error("Error creating audio resampler. Error: {}", ec.message());
                goto afterAudioInit;
            }
        }
    afterAudioInit:

        _setTime(0.0);

        running = true;
        tlog::info("Starting decoder thread");
        decoderThread = std::make_unique<std::thread>(&MediaPlayer::_decoderThreadFunc, this);
        if (audioDecoder.isValid())
        {
            tlog::info("Starting audio player thread");
            audioPlayerThread = std::make_unique<std::thread>(&MediaPlayer::_audioPlayerThreadFunc, this);
        }
        if (videoDecoder.isValid())
        {
            tlog::info("Starting video player thread");
            videoPlayerThread = std::make_unique<std::thread>(&MediaPlayer::_videoPlayerThreadFunc, this);
        }
    }

    void restart() { seek(0.0); }

    void seek(double target)
    {
        LockGuard lg(mutex);
        _seek(target);
    }

    // This will modify the textures data and unpack alignment
    void currentFrameToTexture(Texture& tex)
    {
        // TODO: Copy frame, then unlock, then convert
        // TODO: Cache the frame

        tex.setUnpackAlignment(1);

        //if (!mutex.tryLock()) { return; }
        mutex.lock();
        if (!currentFrame) { mutex.unlock(); return; }

        error_code ec;
        av::VideoFrame outFrame = imgConverter.rescale(currentFrame, ec);
        if (ec)
        { tlog::error("Failed to convert frame: {}", ec.message()); return; }

        ASSERT(outFrame.isValid());
        tex.setData(outFrame.raw()->data[0], videoDecoder.width(), videoDecoder.height(),
                    TextureFiltering::Linear, TexPixelFormats::RGB, TexInternalFormats::RGBA, false);

        mutex.unlock();
    }

    double getDuration() // not const, it locks a mutex
    {
        LockGuard lg(mutex);
        return _getDuration();
    }

    double getPosition() // not const, it locks a mutex
    {
        LockGuard lg(mutex);
        return _getPosition();
    }

    static double getSystemTime()
    { return static_cast<double>(av_gettime()) / static_cast<double>(AV_TIME_BASE); }

    void setPlaying(bool value)
    {
        LockGuard lg(mutex);
        if (value == playing) { return; }
        _setPlaying(value);
    }

    inline bool getPlaying() const { return playing; }

    void setLooping(bool value)
    {
        if (value == looping) { return; }

        looping = value;
    }

    inline bool getLooping() const { return looping; }

    void reset()
    {
        _closeThreads();
        _clearFrameQueue();
        demuxer.close();
        videoPTS     = 0.0;
        audioPTS     = 0.0;
        startTime    = 0.0;
        loopCount    = 0;
        highestAudioPTS = 0;
        highestVideoPTS = 0;
        audioDevice.reset();
    }

    MediaPlayer() = default;
    ~MediaPlayer() { reset(); }

    #pragma region Internal

    struct AudioDevice : NonAssignable
    {
        SDL_AudioDeviceID id  = 0;
        SDL_AudioSpec     specWant{ };
        SDL_AudioSpec     specHave{ };

        AudioDevice() = default;
        ~AudioDevice() { reset(); }

        // False on failure
        bool create(bool startPlaying = true)
        {
            reset();
            id = SDL_OpenAudioDevice(NULL, 0, &specWant, &specHave, 0);
            if (!created()) { return false; }
            if (startPlaying) { setPaused(false); }
            return true;
        }

        void reset()
        {
            if (!created()) { return; }
            SDL_CloseAudioDevice(id);
        }

        void setPaused(bool value)
        {
            ASSERT(created());
            SDL_PauseAudioDevice(id, value);
        }

        void clearQueuedAudio()
        {
            ASSERT(created());
            SDL_ClearQueuedAudio(id);
        }

        void queueAudio(const void* data, Uint32 len)
        {
            ASSERT(created());
            SDL_QueueAudio(id, data, len);
        }

        bool created() const { return id != 0; }
    };

    // Constants
    const size_t maxAudioFrameQueueSize = 128 * 12;
    const size_t maxVideoFrameQueueSize = 64;
    const double audioSyncThreshold     = 0.05;
    const double videoSyncThreshold     = 0.02;
    const int    frequency              = 44100;

    // Thread
    Mutex mutex;
    std::condition_variable_any cond_anyQueueChanged;
    std::condition_variable_any cond_videoQueueChanged;
    std::condition_variable_any cond_audioQueueChanged;
    std::condition_variable_any cond_playing;
    Atomic<bool>   playing       = true;
    Atomic<bool>   running       = false;
    Atomic<bool>   looping       = true;
    Atomic<bool>   eof           = false;
    Atomic<size_t> loopCount     = 0;
    double         startTime     = 0.0;
    double         pauseTime     = 0.0;

    std::unique_ptr<std::thread> decoderThread;
    std::unique_ptr<std::thread> audioPlayerThread;
    std::unique_ptr<std::thread> videoPlayerThread;

    size_t highestVideoPTS = 0;
    size_t highestAudioPTS = 0;

    av::VideoDecoderContext videoDecoder;
    int                     videoStreamID = -1;
    av::Stream              videoStream;

    av::FormatContext       demuxer;

    av::AudioDecoderContext audioDecoder;
    int                     audioStreamID = -1;
    av::Stream              audioStream;
    AudioDevice             audioDevice;
    av::VideoRescaler       imgConverter;
    av::AudioResampler      resampler;
    Atomic<double>          videoPTS = 0.0;
    Atomic<double>          audioPTS = 0.0;
    av::VideoFrame          currentFrame{ };

    using VideoFrameQueue = std::queue<av::VideoFrame, std::deque<av::VideoFrame>>;
    VideoFrameQueue videoFrameQueue;

    using AudioSampleQueue = std::queue<av::AudioSamples, std::deque<av::AudioSamples>>;
    AudioSampleQueue audioFrameQueue;

    inline void _assertMutexLocked(const Mutex& mut) const
    {
        #ifdef _DEBUG
        // If none of the threads are running, it don't matter.
        if (decoderThread || videoPlayerThread || audioPlayerThread)
        {
            ASSERTMSG(mut.lockedByCaller(), "This method isn't thread safe. Lock the thread!");
        }
        #endif
    }

    // Thread unsafe, internal
    double _getTimeSinceStart()
    {
        _assertMutexLocked(mutex);
        return getSystemTime() - startTime;
    }

    double _getDuration() const
    {
        _assertMutexLocked(mutex);
        double dur = demuxer.duration().seconds();
        return dur < 0.0 ? 0.0 : dur;
    }

    double _getPosition() const
    {
        _assertMutexLocked(mutex);
        if      (videoDecoder.isValid()) { return videoPTS; }
        else if (audioDecoder.isValid()) { return audioPTS; }
        return 0.0;
    }

    void _setPlaying(bool value)
    {
        _assertMutexLocked(mutex);
        if (value == false)
        { pauseTime = getSystemTime(); }
        else if (value == true && pauseTime != 0.0)
        {
            startTime += getSystemTime() - pauseTime;
            pauseTime = 0.0;
            if (eof) { _seek(0.0); }
        }

        playing = value;
        cond_playing.notify_all();
    }

    void _seek(double target)
    {
        _assertMutexLocked(mutex);
        const double dur = _getDuration();
        target = std::clamp(target, 0.0, dur);
        loopCount = 0;
        _decoderSeek(target);
        _clearFrameQueue();
        _setTime(target);
        _setPlaying(playing);
    }

    void _decoderSeek(double target)
    {
        _assertMutexLocked(mutex);
        const double pos = _getPosition();

        int flags = AVSEEK_FLAG_ANY;
        if (target < pos) { flags |= AVSEEK_FLAG_BACKWARD; }

        error_code ec;
        demuxer.seek(target * AV_TIME_BASE, -1, flags, ec);
        if (ec)
        {
            tlog::error("Error seeking: {}", ec.message());
        }

        if (videoDecoder.isValid()) { avcodec_flush_buffers(videoDecoder.raw()); }
        if (audioDecoder.isValid()) { avcodec_flush_buffers(audioDecoder.raw()); }
    }

    // Thread unsafe, internal
    void _setTime(double time)
    {
        _assertMutexLocked(mutex);
        double sysTime = getSystemTime();
        startTime = sysTime - time;
    }

    // Thread unsafe, internal
    double _PTSintToDouble(int64_t pts, AVRational timebase) const
    {
        _assertMutexLocked(mutex);
        pts *= av_q2d(timebase);
        return pts;
    }

    #pragma region Frame queue management
    void _clearAudioFrameQueue()
    {
        _assertMutexLocked(mutex);
        if (audioFrameQueue.size() > 0)
        {
            AudioSampleQueue empty;
            audioFrameQueue.swap(empty);
        }
        cond_anyQueueChanged.notify_all();
        cond_audioQueueChanged.notify_all();
    }

    void _clearVideoFrameQueue()
    {
        _assertMutexLocked(mutex);
        //tlog::info("Clearing video frame queue");
        if (videoFrameQueue.size() > 0)
        {
            VideoFrameQueue empty;
            videoFrameQueue.swap(empty);
        }
        cond_anyQueueChanged.notify_all();
        cond_videoQueueChanged.notify_all();
    }

    void _pushVideoQueue(const av::VideoFrame& frame)
    {
        _assertMutexLocked(mutex);
        videoFrameQueue.push(frame);
        cond_anyQueueChanged.notify_all();
        cond_videoQueueChanged.notify_all();
    }

    void _popVideoQueue()
    {
        _assertMutexLocked(mutex);
        videoFrameQueue.pop();
        cond_anyQueueChanged.notify_all();
        cond_videoQueueChanged.notify_all();
    }

    void _pushAudioQueue(const av::AudioSamples& samples)
    {
        _assertMutexLocked(mutex);
        audioFrameQueue.push(samples);
        cond_anyQueueChanged.notify_all();
        cond_audioQueueChanged.notify_all();
    }

    void _popAudioQueue()
    {
        _assertMutexLocked(mutex);
        audioFrameQueue.pop();
        cond_anyQueueChanged.notify_all();
        cond_audioQueueChanged.notify_all();
    }

    void _clearFrameQueue()
    {
        _assertMutexLocked(mutex);
        //tlog::info("Clearing all frame queues");

        _clearVideoFrameQueue();
        _clearAudioFrameQueue();
    }

    #pragma endregion

    #pragma region Threads
    void _closeThreads()
    {
        mutex.lock();
        running = false;
        cond_anyQueueChanged.notify_all();
        cond_videoQueueChanged.notify_all();
        cond_audioQueueChanged.notify_all();
        cond_playing.notify_all();
        mutex.unlock();

        if (decoderThread)
        {
            tlog::info("Closing decoder thread...");
            decoderThread->join();
            decoderThread.release();
        }
        if (audioPlayerThread)
        {
            tlog::info("Closing audio player thread...");
            audioPlayerThread->join();
            audioPlayerThread.release();
        }
        if (videoPlayerThread)
        {
            tlog::info("Closing video player thread...");
            videoPlayerThread->join();
            videoPlayerThread.release();
        }
    }

    Code _decoderThreadFunc()
    {
        while (running)
        {
            Code c = _decodeFrame();
            if (c == EndOfVideoFile)
            {
                LockGuard lg(mutex);
                if (getLooping() && videoFrameQueue.size() == 0 && audioFrameQueue.size() == 0)
                { _seek(0.0); }
            }
            // Fatal error. _decodeFrame will print it.
            else if (c < 0) { return c; }
            else { eof = false; }
        }
        return Success;
    }

    Code _decodeFrame()
    {
        std::unique_lock<Mutex> lock(mutex);

        while (videoFrameQueue.size() >= maxVideoFrameQueueSize ||
               audioFrameQueue.size() >= maxAudioFrameQueueSize)
        {
            cond_anyQueueChanged.wait(lock);
            if (!running) { return Success; }
        }

        error_code ec;
        av::Packet packet = demuxer.readPacket(ec);
        if (ec)
        {
            tlog::error("Error reading packet: {}", ec.message());
            return ErrorReadingPacket;
        }
        else if (!packet.isComplete())
        {
            // Hack: If isComplete is false, its prolly bc of EOF,
            // but libavpp doesn't return any error code when its EOF, so we guess.
            // Error is here in formatcontext.cpp in readPacket:
            // https://github.com/h4tr3d/avcpp/blob/master/src/formatcontext.cpp#L473
            // TLDR: It's a libavpp bug! submit pull request maybe some day zzzz

            // TODO: There's one duplicate frame when looping, fix maybe one day
            // TODO: Also find a way to preview last frame PTS? as an alternative to adding to highestVideoPTS.

            // EOF
            if (highestVideoPTS > 0 || highestAudioPTS > 0) // If the pts of the media doesn't change, it's probably an image
            { return EndOfVideoFile; }
            else { return EndOfImageFile; }
            
        }

        if (packet.pts().timestamp() == AV_NOPTS_VALUE)
        {
            // TODO: handle no pts better?
            // Maybe something like:
            // https://stackoverflow.com/questions/22018784/getting-avframe-pts-value
            // but needs an audio equivalent?
            // Low prio, because im yet to find a media file that makes it a problem
            packet.setPts(av::Timestamp(0, packet.timeBase()));
        }

        int si = packet.streamIndex();

        // Decode video
        if (packet.streamIndex() == videoStreamID)
        {
            if (packet.pts().timestamp() > highestVideoPTS) { highestVideoPTS = packet.pts().timestamp(); }
            packet.setPts(av::Timestamp(packet.pts().timestamp() + (highestVideoPTS * loopCount), packet.timeBase()));

            av::VideoFrame frame = videoDecoder.decode(packet, ec);
            if (ec && ec.value() != AVERROR_EOF)
            {
                tlog::error("Error decoding packet to video: {}", ec.message());
                return ErrorDecodingVideo;
            }

            _pushVideoQueue(frame);
            return DecodedVideo;
        }

        // Decode audio
        else if (packet.streamIndex() == audioStreamID)
        {
            if (packet.pts().timestamp() > highestAudioPTS) { highestAudioPTS = packet.pts().timestamp(); }
            packet.setPts(av::Timestamp(packet.pts().timestamp() + (highestAudioPTS * loopCount), packet.timeBase()));

            av::AudioSamples frame = audioDecoder.decode(packet, ec);
            if (ec && ec.value() != AVERROR_EOF)
            {
                tlog::error("Error decoding packet to audio: {}", ec.message());
                return ErrorDecodingAudio;
            }

            _pushAudioQueue(frame);
            return DecodedAudio;
        }

        return Success;
    }

    int _videoPlayerThreadFunc()
    {
        while (running)
        {
            std::unique_lock<Mutex> lock(mutex);

            while (running && !playing)
            { cond_playing.wait(lock); }

            while (videoFrameQueue.size() <= 0)
            {
                cond_videoQueueChanged.wait(lock);
                if (!running) { return 0; }
                if (!playing) { continue; }
            }

            av::VideoFrame& const frame = videoFrameQueue.front();
            if (!frame)
            {
                tlog::error("Video player received an invalid frame!");
                videoFrameQueue.pop();
                continue;
            }

            auto syncts = _getTimeSinceStart();
            double pts  = frame.pts().seconds();
            videoPTS = pts;

            // We're ahead of the clock, so we wait
            if (pts > syncts + videoSyncThreshold)
            {
                //tlog::info("We're ahead by: {}", pts - syncts);
                continue;
            }

            // We're behind, pop frames to catch up
            else if (pts < syncts - videoSyncThreshold)
            {
                //tlog::info("Popping! We're behind by: {}", abs(pts - syncts));
                frame.swap(currentFrame);
                _popVideoQueue();
            }
        }

        return 0;
    }

    int _audioPlayerThreadFunc()
    {
        error_code ec;

        while (running)
        {
            std::unique_lock<Mutex> lock(mutex);

            while (running && !playing)
            { cond_playing.wait(lock); }

            if (audioFrameQueue.size() <= 0) { continue; }
            av::AudioSamples& frame = audioFrameQueue.front();
            if (!frame) { continue; }

            // Sync
            auto syncts = _getTimeSinceStart();
            double pts = frame.pts().seconds();
            audioPTS = pts;

            // We're ahead of the clock, so we wait
            if (pts > syncts + audioSyncThreshold)
            {
                //tlog::info("We're ahead by: {}", pts - syncts);
                continue;
            }

            // We're behind, start playing!
            else if (pts < syncts - audioSyncThreshold)
            {
                //tlog::info("\nSync ts: {}, TS: {}\nSys time: {}, start time: {}", syncts, pts, getSystemTime(), startTime);
                //tlog::info("Popping! We're behind by: {}", abs(pts - syncts));

                resampler.push(frame, ec);
                if (ec)
                {
                    tlog::error("Error pushing frame too resampler: {}", ec.message());
                    continue;
                }

                av::AudioSamples samples = resampler.pop(0, ec);
                if (ec || samples.isNull())
                {
                    tlog::error("Error popping frame from resampler: {}", ec.message());
                    continue;
                }

                audioDevice.queueAudio(samples.data(), samples.size());
                _popAudioQueue();
            }
        }

        return 0;
    }

    inline void _sleepForMS(size_t ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

    static std::string avErrToStr(int errcode)
    {
        char e[AV_ERROR_MAX_STRING_SIZE];
        return std::string(av_make_error_string(e, AV_ERROR_MAX_STRING_SIZE, errcode));
    }
    #pragma endregion

    #pragma endregion
};