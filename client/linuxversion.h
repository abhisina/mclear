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
#include <optional>
#include <ostream>
#include <string>
#include <initializer_list>

class LinuxVersion
{
    uint8_t major_;
    uint8_t minor_;

public:
    LinuxVersion(uint8_t major = 0, uint8_t minor = 0) : major_(major), minor_(minor) {}
    LinuxVersion(const std::initializer_list<uint8_t>& version);

    static std::optional<LinuxVersion> fromStr(const std::string& release);
    bool operator<(const LinuxVersion& r) const;
    bool operator<=(const LinuxVersion& r) const;
    bool operator>(const LinuxVersion& r) const;
    bool operator>=(const LinuxVersion& r) const;
    bool operator==(const LinuxVersion& r) const;
    bool operator!=(const LinuxVersion& r) const;

    friend std::ostream& operator<<(std::ostream& out, const LinuxVersion& version);
};
