#pragma once

#include <chrono>

struct Time : std::chrono::duration<double>
{
    using nanoseconds  = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds      = std::chrono::seconds;
    using minutes      = std::chrono::minutes;
    using hours        = std::chrono::hours;

    Time() = default;
    Time(const std::chrono::duration<double>& base) : std::chrono::duration<double>(base) { }
    Time& operator=(const std::chrono::duration<double>& base) { std::chrono::duration<double>::operator=(base); return *this; }

    inline double asNanoseconds() const noexcept
    { return std::chrono::duration_cast<nanoseconds>(*this).count(); }

    inline double asMicroseconds() const noexcept
    { return std::chrono::duration_cast<microseconds>(*this).count(); }

    inline double asMilliseconds() const noexcept
    { return std::chrono::duration_cast<milliseconds>(*this).count(); }

    inline double asSeconds() const noexcept
    { return count(); }
};

class Timer
{
public:
    using TimePoint    = std::chrono::steady_clock::time_point;
    using nanoseconds  = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds      = std::chrono::seconds;
    using minutes      = std::chrono::minutes;
    using hours        = std::chrono::hours;

    Timer(bool paused = false) noexcept
    {
        restart();
        setPaused(paused);
    }

    inline Time getElapsedTime() const
    {
        if (_paused)
        { return Time(_pausedTime - _startTime); }
        else
        { return Time(std::chrono::steady_clock::now() - _startTime); }
    }

    inline Time restart()
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

    static inline TimePoint now()
    { return std::chrono::steady_clock::now(); }

    inline const TimePoint getStartTime() const noexcept
    { return _startTime; }

    bool _paused = false;
    TimePoint _pausedTime;
    TimePoint _startTime;
};