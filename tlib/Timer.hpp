#pragma once

#include <chrono>

struct Time : std::chrono::nanoseconds
{
    using nanoseconds  = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds      = std::chrono::seconds;
    using minutes      = std::chrono::minutes;
    using hours        = std::chrono::hours;

    Time() = default;
    Time(const nanoseconds& base) : nanoseconds::duration(base) { }
    Time& operator=(const nanoseconds& base) { nanoseconds::operator=(base); return *this; }

    inline double asNanoseconds() noexcept
    { return count(); }

    inline double asMicroseconds() noexcept
    { return std::chrono::duration_cast<microseconds>(*this).count(); }

    inline double asMilliseconds() noexcept
    { return std::chrono::duration_cast<milliseconds>(*this).count(); }

    inline double asSeconds() noexcept
    { return std::chrono::duration_cast<seconds>(*this).count(); }

    inline double asMinutes() noexcept
    { return std::chrono::duration_cast<minutes>(*this).count(); }

    inline double asHours() noexcept
    { return std::chrono::duration_cast<hours>(*this).count(); }
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
        return Time(endTime - startTime);
    }

    Time restart() noexcept
    {
        auto t = getElapsedTime();
        startTime = now();
        return t;
    }

    static inline TimePoint now() noexcept
    { return std::chrono::steady_clock::now(); }

    inline const TimePoint getStartTime() const noexcept
    { return startTime; }

protected:
    TimePoint startTime;
};