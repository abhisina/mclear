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

#include <catch2/catch_test_macros.hpp>
#include "linuxversion.h"
#include "adjustoomscore.h"
#include "uname.h"
#include "memoryinfo.h"

#include <filesystem>
#include <fstream>

TEST_CASE("Linux Version Test", "[LinuxVersion]")
{
    SECTION("check if the string is being parsed correctly")
    {
        REQUIRE(*LinuxVersion::fromStr("3.8.3-Fedora") == LinuxVersion{3, 8});
        REQUIRE(LinuxVersion::fromStr("5.16.18-1-generic").value() == LinuxVersion{5, 16});
    }

    SECTION("check the version string comparisions")
    {
        // std optional checks if the underlying value is set then it dispatched the operator call to the
        // underlying objects operator overload function
        REQUIRE(LinuxVersion::fromStr("3.8.3") >= LinuxVersion::fromStr("3.6.9"));

        REQUIRE(LinuxVersion::fromStr("3.8.3").value() == LinuxVersion::fromStr("3.8.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() != LinuxVersion::fromStr("3.8.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() < LinuxVersion::fromStr("5.8.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() <= LinuxVersion::fromStr("3.10.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() <= LinuxVersion::fromStr("3.9.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() > LinuxVersion::fromStr("3.8.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() >= LinuxVersion::fromStr("3.8.9").value());
        REQUIRE(LinuxVersion::fromStr("3.9.3").value() >= LinuxVersion::fromStr("3.9.9").value());

        REQUIRE_FALSE(LinuxVersion::fromStr("393").has_value());
    }
}

TEST_CASE("check if oom score is being set correctly", "[AdjustOomScore]")
{
    std::FILE* tmpf = std::tmpfile();
    auto filePath = std::filesystem::read_symlink(std::filesystem::path("/proc/self/fd") / std::to_string(fileno(tmpf)));
    const char* oomfile = filePath.c_str();
    {
        std::ofstream file(oomfile);
        file << -1000 << '\n';
    }
    int newScore = -500;
    AdjOOMScore::adjust(newScore, oomfile);
    
    int score = [&oomfile](){
        std::ifstream file(oomfile);
        int score = 0;
        file >> score;
        return score;
    }();

    REQUIRE(score == newScore);
    fclose(tmpf);
    std::filesystem::remove(filePath);
}

TEST_CASE("check linux version generation", "[Uname]")
{
    SECTION("check default argument")
    {
        REQUIRE(Uname("5.4.0-110-generic").parseVersion() == LinuxVersion{5, 4});
    }

    SECTION("check actual version parsing")
    {
        utsname utsStruct;
        REQUIRE_FALSE(uname(&utsStruct));
        REQUIRE(LinuxVersion::fromStr(utsStruct.release) == Uname().parseVersion());
    }
}

TEST_CASE("check memory info parsing", "[MemoryInfo]")
{
    auto writePressureStallInfoFile = [](const char* pressureStallInfoFile){
        std::ofstream file(pressureStallInfoFile);
        file << "some avg10=23.56 avg60=0.00 avg300=0.00 total=109101\n";
        file << "full avg10=0.00 avg60=0.00 avg300=0.00 total=80290\n";
    };

    SECTION("check pessure info for systems < Linux 4.20")
    {
        std::FILE* tmpf = std::tmpfile();
        auto filePath = std::filesystem::read_symlink(std::filesystem::path("/proc/self/fd") / std::to_string(fileno(tmpf)));

        const char* pressureStallInfoFile = filePath.c_str();
        writePressureStallInfoFile(pressureStallInfoFile);

        MemoryInfo info(pressureStallInfoFile, "4.1.0-110-MANJARO");

        REQUIRE(info.avg10ProcessStallTime() == -1.f);

        REQUIRE(info.update());
        REQUIRE(info.avg10ProcessStallTime() == -1.f);

        fclose(tmpf);
        std::filesystem::remove(filePath);
    }

    SECTION("check pressure stall info for others")
    {
        std::FILE* tmpf = std::tmpfile();
        auto filePath = std::filesystem::read_symlink(std::filesystem::path("/proc/self/fd") / std::to_string(fileno(tmpf)));

        const char* pressureStallInfoFile = filePath.c_str();
        writePressureStallInfoFile(pressureStallInfoFile);

        MemoryInfo info(pressureStallInfoFile, "5.4.0-110-generic");

        REQUIRE(info.avg10ProcessStallTime() == 0.f);

        REQUIRE(info.update());
        REQUIRE(info.avg10ProcessStallTime() == 23.56f);

        fclose(tmpf);
        std::filesystem::remove(filePath);
    }
}
