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

    Time getElapsedTime() const noexcept
    {
        const auto endTime = std::chrono::steady_clock::now();
        return Time(endTime - _startTime);
    }

    Time restart() noexcept
    {
        auto t = getElapsedTime();
        _startTime = now();
        return t;
    }

    static inline TimePoint now() noexcept
    { return std::chrono::steady_clock::now(); }

    inline const TimePoint getStartTime() const noexcept
    { return _startTime; }

    TimePoint _startTime;
};