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

#include <iostream>

#include <mcproto/networkinfo.grpc.pb.h>
#include <mcproto/cpuloadinfo.grpc.pb.h>
#include <mcproto/diskinfo.grpc.pb.h>
#include <mcproto/memoryinfo.grpc.pb.h>
#include <mcproto/infoupdate.grpc.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/server_builder.h>

#include <queue>
#include <mutex>

class InfoUpdateService final : public mcproto::InfoUpdate::Service
{
    struct Stat
    {
        Stat():cpuIdlePercent(0.f), diskSpaceAvailable(0), networkBandwidthUsed(0), ramUsedPercent(0), swapUsedPercent(0)
        {}

        std::string hostanme;
        float cpuIdlePercent;
        unsigned long diskSpaceAvailable;
        unsigned int networkBandwidthUsed;
        unsigned char ramUsedPercent;
        unsigned char swapUsedPercent;
    };

    struct compare
    {
        bool operator()(const Stat& l, const Stat& r)
        {
            // TODO factor out ordering based on different stats
            return false;
        }
    };

    std::priority_queue<Stat, std::vector<Stat>, compare> nodes;
    std::unordered_set<std::string> hosts;
    std::mutex protect;

public:
    virtual ::grpc::Status SendStats(::grpc::ServerContext* context, const ::mcproto::Stats* request, ::mcproto::Empty* response) override
    {
        Stat stat;
        std::cout << "hostname: " << request->hostname() << '\n';
        stat.hostanme = request->hostname();

        if(request->has_cpuload())
        {
            std::cout << "Cpu Load(%): " << double(request->cpuload().cpuload()) / 100. << '\n';
            stat.cpuIdlePercent = 100. - double(request->cpuload().cpuload()) / 100.;
        }

        if(request->has_diskinfo())
        {
            std::cout << "DiskSpace(KB): " << request->diskinfo().availablespace() << '\n';
            stat.diskSpaceAvailable = request->diskinfo().availablespace();
        }

        if(request->has_netinfo())
        {
            std::cout << "Network Speed(bytes/sec): " << request->netinfo().bandwidthusage() << '\n';
            stat.networkBandwidthUsed = request->netinfo().bandwidthusage();
        }

        if(request->has_meminfo())
        {
            std::cout << "Memory Info:\n";
            std::cout << "Available Ram(MB): " << request->meminfo().availableram() << '\n';
            std::cout << "Available Swap(MB): " << request->meminfo().availableswap() << '\n';
            std::cout << "Available Ram(%): " << request->meminfo().availablerampercent() << '\n';
            std::cout << "Available Swap(%): " << request->meminfo().availableswappercent() << '\n';
            std::cout << "Avg10 stall info(us): " << request->meminfo().avg10processstalltime() << '\n';
            stat.ramUsedPercent = request->meminfo().availablerampercent();
            stat.swapUsedPercent = request->meminfo().availableswappercent();
        }

        std::cout << "============================================================" << std::endl;

        std::lock_guard<std::mutex> guard(protect);
        // TODO: factor out existing node
        if(hosts.find(stat.hostanme) != hosts.end())
        {
            hosts.insert(stat.hostanme);
            nodes.push(stat);
        }
        else
        {
            // TODO:
        }

        return grpc::Status::OK;
    }
};

int main()
{
    grpc::ServerBuilder builder;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());

    InfoUpdateService service;
    builder.RegisterService(&service);

    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    server->Wait();
}