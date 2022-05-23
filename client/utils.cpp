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

#include "utils.h"
#include <unistd.h>

#include <iostream>
#include "cpuloadinfo.h"
#include "networkinfo.h"
#include "diskspaceinfo.h"
#include "memoryinfo.h"

#include <mcproto/networkinfo.grpc.pb.h>
#include <mcproto/cpuloadinfo.grpc.pb.h>
#include <mcproto/diskinfo.grpc.pb.h>
#include <mcproto/memoryinfo.grpc.pb.h>
#include <mcproto/infoupdate.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/create_channel.h>

#include "adjustoomscore.h"
#include "daemonize.h"
#include "lockmemory.h"

uint32_t Utils::effectiveUserId()
{
    return geteuid();
}

bool Utils::runningAsSudo()
{
    return !effectiveUserId();
}

void Utils::initializeService()
{
    Daemonize::initiate("mclearcli.out", "mclearcli.err", "mclearcli.pid");
    std::cout << "Daemonize suceeded!" << std::endl;

    //LockMemory::lock();
    //std::cout << "Memory locking succeeded" << std::endl;
    AdjOOMScore::adjust(-500);
    std::cout << "oom score adjust succeeded" << std::endl;
}

void Utils::run(uint32_t sec)
{
    std::cout << "Starting runloop with sleep time:" << sec << std::endl;

    char hostname[HOST_NAME_MAX + 1] = {};
    gethostname(hostname, sizeof(hostname));
    NetworkInfo netinfo;
    CpuLoadInfo cpuinfo;
    DiskSpaceInfo diskinfo;
    MemoryInfo meminfo;

    auto channel = grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials());
    std::unique_ptr<mcproto::InfoUpdate::Stub> stub = mcproto::InfoUpdate::NewStub(channel);
    mcproto::Empty result;

    mcproto::Stats stats;
    mcproto::DiskInfo* protodiskinfo = new mcproto::DiskInfo;
    mcproto::CpuLoadInfo* protocpuloadinfo = new mcproto::CpuLoadInfo;
    mcproto::NetworkInfo* protonetworkinfo = new mcproto::NetworkInfo;
    mcproto::MemoryInfo* protomemoryinfo = new mcproto::MemoryInfo;
    stats.set_allocated_cpuload(protocpuloadinfo);
    stats.set_allocated_netinfo(protonetworkinfo);
    stats.set_allocated_meminfo(protomemoryinfo);
    stats.set_allocated_diskinfo(protodiskinfo);
    stats.set_hostname(hostname);

    for(;;)
    {
        sleep(sec);

        if(!diskinfo.update())
        {
            protodiskinfo->Clear();
            std::cerr << "DiskInfo Update failed\n";
        }
        else
        {
            protodiskinfo->set_availablespace(diskinfo.availableSpace());            
        }

        if(!meminfo.update())
        {
            protomemoryinfo->Clear();
            std::cerr << "MemInfo update failed\n";
        }
        else
        {
            protomemoryinfo->set_availableswap(meminfo.availableSwap());
            protomemoryinfo->set_availableram(meminfo.availableRam());
            protomemoryinfo->set_availablerampercent(meminfo.availableRamPercent());
            protomemoryinfo->set_availableswappercent(meminfo.availableSwapPercent());
            protomemoryinfo->set_avg10processstalltime(meminfo.avg10ProcessStallTime());
        }

        protocpuloadinfo->set_cpuload(cpuinfo.cpuLoad());
        protonetworkinfo->set_bandwidthusage(netinfo.bandwidthUsage());

        grpc::ClientContext context;
        grpc::Status status = stub->SendStats(&context, stats, &result);
        
        if(!status.ok())
        {
            // TODO: send the update to the backup server
            std::cerr << "rpc failed!\n";
        }
    }
}
