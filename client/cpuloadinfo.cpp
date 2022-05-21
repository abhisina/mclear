/*
 * Copyright (c) 2022 Abhinav Sinha
 *
 * This work can be distributed under the terms of the GNU GPLv3.
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License Version 3 for more details.
 */

#include "cpuloadinfo.h"

#include <chrono>
#include <thread>
#include <vector>
#include <fstream>
#include <numeric>

namespace
{
    inline std::vector<size_t> getCpuTimes()
    {
        std::vector<size_t> times;
        {
            std::ifstream procStat("/proc/stat");
            procStat.ignore(5, ' '); // skip the 'cpu ' prefix
            for(size_t time ; procStat >> time ; times.push_back(time));
        }
        return times;
    }

    inline bool getCpuTimes(size_t& idleTime, size_t& totalTime)
    {
        const std::vector<size_t> cpuTimes = getCpuTimes();
        if(cpuTimes.size() < 4)
            return false;
        
        idleTime = cpuTimes[3];
        totalTime = std::accumulate(cpuTimes.begin(), cpuTimes. end(), 0);
        return true;
    }

    void update(uint32_t& cpuLoad)
    {
        using namespace std::chrono_literals;
        size_t previousIdleTime = 0;
        size_t previousTotalTime = 0;
        for(size_t idleTime, totalTime ; getCpuTimes(idleTime, totalTime) ; std::this_thread::sleep_for(5s))
        {
            const size_t idleTimeDelta = idleTime - previousIdleTime;
            const size_t totalTimeDelta = totalTime - previousTotalTime;
            cpuLoad = 10000. * (1. - double(idleTimeDelta) / totalTimeDelta);
            previousIdleTime = idleTime;
            previousTotalTime = totalTime;
        }
    }
}

CpuLoadInfo::CpuLoadInfo(): cpuLoad_(0.f)
{
    std::thread(update, std::ref(cpuLoad_)).detach();
}

CpuLoadInfo::~CpuLoadInfo()
{}
