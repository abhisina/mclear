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

#include <string>

class Daemonize
{
    int pidFd_;
    Daemonize(const std::string& outFile, const std::string& errFile, const std::string& pidFile);

public:
    static Daemonize& initiate(const std::string& outFile, const std::string& errFile, const std::string& pidFile);

    /// runtime should call this destructor at exit as it does for static objects
    ~Daemonize();
};