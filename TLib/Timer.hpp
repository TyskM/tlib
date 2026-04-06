#pragma once

#include <chrono>

using Time         = std::chrono::steady_clock::time_point;
using Clock        = std::chrono::steady_clock;
using Nanoseconds  = std::chrono::nanoseconds;
using Microseconds = std::chrono::microseconds;
using Milliseconds = std::chrono::milliseconds;
using Seconds      = std::chrono::seconds;
using Minutes      = std::chrono::minutes;
using Hours        = std::chrono::hours;

struct Duration : std::chrono::duration<double>
{
    Duration() = default;
    Duration(const std::chrono::duration<double>& base) : std::chrono::duration<double>(base) { }
    Duration& operator=(const std::chrono::duration<double>& base) { std::chrono::duration<double>::operator=(base); return *this; }

    inline double asNanoseconds() const noexcept
    { return static_cast<double>(std::chrono::duration_cast<Nanoseconds>(*this).count()); }

    inline double asMicroseconds() const noexcept
    { return static_cast<double>(std::chrono::duration_cast<Microseconds>(*this).count()); }

    inline double asMilliseconds() const noexcept
    { return static_cast<double>(std::chrono::duration_cast<Milliseconds>(*this).count()); }

    inline double asSeconds() const noexcept
    { return count(); }
};

class Timer
{
public:
    Timer(bool paused = false) noexcept
    {
        restart();
        setPaused(paused);
    }

    inline Duration getElapsedTime() const
    {
        if (_paused)
        { return Duration(_pausedTime - _startTime); }
        else
        { return Duration(std::chrono::steady_clock::now() - _startTime); }
    }

    inline Duration restart()
    {
        auto t = getElapsedTime();
        _startTime = now();
        return t;
    }

    inline void setPaused(bool v = true)
    {
        if (v == _paused) { return; }
        _paused = v;
        if (v) { _pausedTime = now(); }
        else   { _startTime += now() - _pausedTime; }
    }

    inline bool getPaused() const noexcept { return _paused; }

    static inline Time now()
    { return std::chrono::steady_clock::now(); }

    inline const Time getStartTime() const noexcept
    { return _startTime; }

    bool _paused = false;
    Time _pausedTime;
    Time _startTime;
};