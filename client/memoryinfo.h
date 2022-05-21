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

#pragma once

#include <cstdint>
#include <ostream>

class MemoryInfo
{
    uint64_t availableRamMb_;
    uint64_t availableSwapMb_;
    uint8_t availableRamPercent_;
    uint8_t availableSwapPercent_;
    float avg10ProcessStallTimeUs_;
    const char* pressureInfoFile_;

public:
    MemoryInfo(const char* pressurefile = "/proc/pressure/memory", const char* release = 0);
    ~MemoryInfo();

    bool update();
    uint64_t availableRam() const { return availableRamMb_; }
    uint64_t availableSwap() const { return availableSwapMb_; }
    uint8_t availableRamPercent() const { return availableRamPercent_; }
    uint8_t availableSwapPercent() const { return availableSwapPercent_; }
    float avg10ProcessStallTime() const { return avg10ProcessStallTimeUs_; }

    friend std::ostream& operator<<(std::ostream& out, const MemoryInfo& info);
};