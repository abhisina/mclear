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

#include "uname.h"

#include <string.h>
#include <system_error>

Uname::Uname(const char* release)
{
    if(release)
    {
        memset(&utsStruct_, 0, sizeof(utsStruct_));
        strncpy(utsStruct_.release, release, sizeof(utsStruct_.release));
    }
    else if(uname(&utsStruct_))
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }
}

LinuxVersion Uname::parseVersion() const
{
    return LinuxVersion::fromStr(utsStruct_.release).value();
}

std::ostream& operator<<(std::ostream& out, const Uname& uname)
{
    out << "OS:           " << uname.utsStruct_.sysname << '\n';
    out << "Hostname:     " << uname.utsStruct_.nodename << '\n';
    out << "Version:      " << uname.utsStruct_.release << '\n';
    out << "Architecture: " << uname.utsStruct_.machine << '\n';
    return out;
}
