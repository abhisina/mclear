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

#include "diskspaceinfo.h"

#include <filesystem>
#include <iostream>

DiskSpaceInfo::DiskSpaceInfo(const char* diskMount):
    availableDiskSpaceKb_(0),
    diskMountPath_(diskMount)
{}

DiskSpaceInfo::~DiskSpaceInfo()
{}

bool DiskSpaceInfo::update()
{
    std::error_code ec;
    auto spaceInfo = std::filesystem::space(diskMountPath_, ec);
    if(spaceInfo.available == -1)
    {
        std::cerr << "could not calculate disk space info\n";
        return false;
    }

    availableDiskSpaceKb_ = spaceInfo.available / 1000;
    return true;
}
