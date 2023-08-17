#pragma once

// Windows only for now

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "psapi.h"
#include "processthreadsapi.h"
#endif

#include <TLib/Timer.hpp>
#include <cstdint>

// https://stackoverflow.com/questions/63166/how-to-determine-cpu-and-memory-consumption-from-inside-a-process

namespace sysq
{
    namespace detail
    {
    #ifdef _WIN32
        // For getThisProcessCPUUsage()
        FILETIME prevSysKernel;
        FILETIME prevSysUser;
        FILETIME prevProcKernel;
        FILETIME prevProcUser;
        short cpuUsage;
        ULONGLONG lastRun;
        volatile LONG runCount;
        Timer timer;
        double cacheTime = 200; // MS
        bool inited = false;

        ULONGLONG subtractTimes(const FILETIME& ftA, const FILETIME& ftB)
        {
            LARGE_INTEGER a{}, b{};

            a.LowPart = ftA.dwLowDateTime;
            a.HighPart = ftA.dwHighDateTime;

            b.LowPart = ftB.dwLowDateTime;
            b.HighPart = ftB.dwHighDateTime;

            return a.QuadPart - b.QuadPart;
        }
    #endif
    }

    uint64_t bytesToMb(uint64_t value)
    {
        auto kb = value >> 10;
        auto mb = kb >> 10;
        return mb;
    }

    uint64_t kbToMb(uint64_t value)
    {
        auto mb = value >> 10;
        return mb;
    }

    // Values are in bytes
    struct GlobalMemoryInfo
    {
        bool success = false;
        uint64_t totalPhysical;
        uint64_t availPhysical;
        uint64_t totalPageFile;
        uint64_t availPageFile;
        uint64_t totalVirtual;
        uint64_t availVirtual;
        uint64_t availExtendedVirtual;
    };

    // Values are in bytes
    struct PrivateMemoryInfo
    {
        bool success = false;
        uint32_t pageFaultCount;
        uint64_t peakWorkingSetSize;
        uint64_t workingSetSize; // This is the physical memory used by your process (in bytes)
        uint64_t quotaPeakPagedPoolUsage;
        uint64_t quotaPagedPoolUsage;
        uint64_t quotaPeakNonPagedPoolUsage;
        uint64_t quotaNonPagedPoolUsage;
        uint64_t pagefileUsage;
        uint64_t peakPagefileUsage;
        uint64_t privateUsage; // This is the virtual memory used by your process (in bytes)
    };

    const GlobalMemoryInfo getGlobalMemInfo()
    {
        GlobalMemoryInfo gbi;

    #ifdef _WIN32
        MEMORYSTATUSEX memInfo{};
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);

        if (GlobalMemoryStatusEx(&memInfo))
        {
            gbi.totalPhysical = memInfo.ullTotalPhys;
            gbi.availPhysical = memInfo.ullAvailPhys;
            gbi.totalPageFile = memInfo.ullTotalPageFile;
            gbi.availPageFile = memInfo.ullAvailPageFile;
            gbi.totalVirtual = memInfo.ullTotalVirtual;
            gbi.availVirtual = memInfo.ullAvailVirtual;
            gbi.availExtendedVirtual = memInfo.ullAvailExtendedVirtual;
            gbi.success = true;
            return gbi;
        }
    #endif

        return gbi;
    }

    const PrivateMemoryInfo getThisProcessMemUsage()
    {
        PrivateMemoryInfo lmi;

    #ifdef _WIN32
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
        {
            lmi.pageFaultCount             = pmc.PageFaultCount;
            lmi.peakWorkingSetSize         = pmc.PeakWorkingSetSize;
            lmi.workingSetSize             = pmc.WorkingSetSize;
            lmi.quotaPeakPagedPoolUsage    = pmc.QuotaPeakPagedPoolUsage;
            lmi.quotaPagedPoolUsage        = pmc.QuotaPagedPoolUsage;
            lmi.quotaPeakNonPagedPoolUsage = pmc.QuotaPeakNonPagedPoolUsage;
            lmi.quotaNonPagedPoolUsage     = pmc.QuotaNonPagedPoolUsage;
            lmi.pagefileUsage              = pmc.PagefileUsage;
            lmi.peakPagefileUsage          = pmc.PeakPagefileUsage;
            lmi.privateUsage               = pmc.PrivateUsage;
            lmi.success = true;
        }
    #endif

        return lmi;
    }

    short getThisProcessCPUUsage()
    {
        // http://www.philosophicalgeek.com/2009/01/03/determine-cpu-usage-of-current-process-c-and-c/

    #ifdef _WIN32
        short cpuCopy = detail::cpuUsage;

        if (InterlockedIncrement(&detail::runCount) == 1)
            {
                if (detail::timer.getElapsedTime().asMilliseconds() < detail::cacheTime)
                {
                    InterlockedDecrement(&detail::runCount);
                    return cpuCopy;
                }

                FILETIME sysIdle, sysKernel, sysUser;
                FILETIME procCreation, procExit, procKernel, procUser;

                if (!GetSystemTimes(&sysIdle, &sysKernel, &sysUser) ||
                    !GetProcessTimes(GetCurrentProcess(), &procCreation, &procExit, &procKernel, &procUser))
                {
                    InterlockedDecrement(&detail::runCount);
                    return cpuCopy;
                }

                if (!(detail::lastRun == 0))
                {
                    ULONGLONG sysKernelDiff = detail::subtractTimes(sysKernel, detail::prevSysKernel);
                    ULONGLONG sysUserDiff = detail::subtractTimes(sysUser, detail::prevSysUser);
                    ULONGLONG procKernelDiff = detail::subtractTimes(procKernel, detail::prevProcKernel);
                    ULONGLONG procUserDiff = detail::subtractTimes(procUser, detail::prevProcUser);
                    ULONGLONG totalSys =  sysKernelDiff + sysUserDiff;
                    ULONGLONG totalProc = procKernelDiff + procUserDiff;

                    if (totalSys > 0)
                    {
                        detail::cpuUsage = (short)((100.0 * totalProc) / totalSys);
                    }
            
                }

                detail::prevSysKernel  = sysKernel;
                detail::prevSysUser    = sysUser;
                detail::prevProcKernel = procKernel;
                detail::prevProcUser   = procUser;

                detail::lastRun = GetTickCount64();

                detail::cpuUsage;
                detail::timer.restart();
            }

        InterlockedDecrement(&detail::runCount);

        return cpuCopy;

    #else
        return 0;
    
    #endif
    }
}