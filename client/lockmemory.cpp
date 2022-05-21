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

#include "lockmemory.h"
#include <sys/mman.h>

#include <system_error>

LockMemory::LockMemory()
{
    if(mlockall(MCL_CURRENT|MCL_FUTURE|MCL_ONFAULT) == -1)
        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1)
            throw std::system_error(std::error_code(errno, std::system_category()));
}

LockMemory::~LockMemory()
{}

void LockMemory::lock()
{
    LockMemory locked;
}