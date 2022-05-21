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

#include "linuxversion.h"
#include <limits>

namespace
{
    std::optional<uint8_t> revisionHelper(const std::string& version)
    {
        try
        {
            unsigned long revision = std::stoul(version);
            if(revision > std::numeric_limits<uint8_t>::max())
                return {};
            return uint8_t(revision);
        }
        catch(const std::invalid_argument&)
        {}
        catch(const std::out_of_range&)
        {}

        return {};        
    }
}

std::ostream& operator<<(std::ostream& out, const LinuxVersion& version)
{
    return out << "Linux " << int(version.major_) << '.' << int(version.minor_);
}

LinuxVersion::LinuxVersion(const std::initializer_list<uint8_t>& version)
{
    auto iterator = version.begin();
    major_ = *iterator;
    iterator++;
    minor_ = *iterator;
}

/// Given a relase string as given bu uname -r, attempt
/// to extract the major and minor values of linux version
/// $ uname -r
/// 5.4.0-110-generic
std::optional<LinuxVersion> LinuxVersion::fromStr(const std::string& release)
{
    auto dot_idx = release.find('.');
    if(dot_idx == std::string::npos)
        return {};
    auto major = revisionHelper(release.substr(0, dot_idx));
    if(!major)
        return {};
    auto dot_2idx = release.find('.', dot_idx + 1);
    if(dot_2idx == std::string::npos)
        return {};
    auto minor = revisionHelper(release.substr(dot_idx + 1, dot_2idx - dot_idx - 1));
    if(!minor)
        return {};
    return LinuxVersion{*major, *minor};
}

bool LinuxVersion::operator<(const LinuxVersion& ver) const
{
    return major_ < ver.major_ || (major_ == ver.major_ && minor_ < ver.minor_);
}

bool LinuxVersion::operator<=(const LinuxVersion& ver) const
{
    return *this < ver || *this == ver;
}

bool LinuxVersion::operator>(const LinuxVersion& ver) const
{
    return !(*this < ver);
}

bool LinuxVersion::operator>=(const LinuxVersion& ver) const
{
    return *this > ver || *this == ver;
}

bool LinuxVersion::operator==(const LinuxVersion& ver) const
{
    return major_ == ver.major_ && minor_ == ver.minor_;
}

bool LinuxVersion::operator!=(const LinuxVersion& ver) const
{
    return !(*this == ver);
}