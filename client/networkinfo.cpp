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

#include "networkinfo.h"

#include <thread>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>

namespace
{
    void update(uint32_t& bps)
    {
        // find the default interface
        // TODO: change interface dynamically when the default interface changes
        std::string interface;
        {
            std::ifstream file("/proc/net/route");
            std::string destination;
            while(file >> interface >> destination)
            {
                if(destination == "00000000")
                    break;
                file.ignore(100, '\n');
            }
        }

        uint64_t prevRx = 0, prevTx = 0;
        for(;;)
        {
            {
                std::ifstream file("/proc/net/dev");
                std::string line;
                while(std::getline(file, line))
                {
                    if(line.find(interface) != std::string::npos)
                    {
                        size_t pos = line.find(':');
                        if(pos != std::string::npos)
                        {
                            uint64_t rx, tx, tmp;
                            std::istringstream stream(line.substr(pos + 1));
                            stream >> rx;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tmp;
                            stream >> tx;
                            bps = ((rx - prevRx) + (tx - prevTx)) / 5;
                            prevRx = rx;
                            prevTx = tx;
                        }
                        break;
                    }
                }
            }
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(5s);
        }
    }
}

NetworkInfo::NetworkInfo()
{
    std::thread(update, std::ref(bandwidthUsageBps_)).detach();
}

NetworkInfo::~NetworkInfo()
{}
