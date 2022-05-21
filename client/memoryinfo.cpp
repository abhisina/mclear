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

#include "memoryinfo.h"
#include "uname.h"

#include <iostream>
#include <fstream>
#include <sys/sysinfo.h>

MemoryInfo::MemoryInfo(const char* pressurefile, const char* release):
    availableRamMb_(0),
    availableSwapMb_(0),
    availableRamPercent_(0),
    availableSwapPercent_(0),
    avg10ProcessStallTimeUs_(0.f),
    pressureInfoFile_(pressurefile)
{
    /// pressure stall information was introduced in Linux 4.20
    if(Uname(release).parseVersion() < LinuxVersion{4, 20})
    {
        std::cout << "Pressure Stall Information not available\n";
        avg10ProcessStallTimeUs_ = -1.f;
    }
}

MemoryInfo::~MemoryInfo()
{}

namespace
{
    inline uint64_t memSizeToMegaBytes(uint64_t size, uint32_t memUnit)
    {
        return size / (1000 * 1000) * memUnit;
    }

    inline uint8_t percentage(uint64_t part, uint64_t total)
    {
        return total ? uint8_t(double(part) / double(total) * 100.) : 0;
    }
}

bool MemoryInfo::update()
{
    struct sysinfo info;
    if(sysinfo(&info) == -1)
    {
        std::cerr << "call to sysinfo failed:" << errno << '\n';
        return false;
    }

    availableRamMb_ = memSizeToMegaBytes(info.freeram, info.mem_unit);
    availableSwapMb_ = memSizeToMegaBytes(info.freeswap, info.mem_unit);
    const uint64_t totalRamMb = memSizeToMegaBytes(info.totalram, info.mem_unit);
    const uint64_t totalSwapMb = memSizeToMegaBytes(info.totalswap, info.mem_unit);
    availableRamPercent_ = percentage(availableRamMb_, totalRamMb);
    availableSwapPercent_ = percentage(availableSwapMb_, totalSwapMb);

    if(avg10ProcessStallTimeUs_ != -1.f)
    {
        // read from pressureInfoFile_ file with format:
        // some avg10=0.00 avg60=0.00 avg300=0.00 total=98028
        // full avg10=0.00 avg60=0.00 avg300=0.00 total=74503
        std::ifstream pressInfo(pressureInfoFile_);
        pressInfo.ignore(11, '=');
        pressInfo >> avg10ProcessStallTimeUs_;
    }

    return true;
}

std::ostream& operator<<(std::ostream& out, const MemoryInfo& info)
{
    out << "Available Ram(MB)   :" << info.availableRamMb_ << '\n';
    out << "Available Ram(%)    :" << int(info.availableRamPercent_) << '\n';
    out << "Available Swap(MB)  :" << info.availableSwapMb_ << '\n';
    out << "Available Swap(%)   :" << int(info.availableSwapPercent_) << '\n';
    out << "Avg10 stall time(us):" << info.avg10ProcessStallTimeUs_ << '\n';
    return out;
}