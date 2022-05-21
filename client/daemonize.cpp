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

#include "daemonize.h"
#include "utils.h"

#include <system_error>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

/// http://web.archive.org/web/20120914180018/http://www.steve.org.uk/Reference/Unix/faq_2.html#SEC16
/// Here are the steps to become a daemon:
/// 1. fork() so the parent can exit, this returns control to the command line or shell invoking your program. This step is required so that the new process is guaranteed not to be a process group leader. The next step, setsid(), fails if you're a process group leader.
/// 2. setsid() to become a process group and session group leader. Since a controlling terminal is associated with a session, and this new session has not yet acquired a controlling terminal our process now has no controlling terminal, which is a Good Thing for daemons.
/// 3. fork() again so the parent, (the session group leader), can exit. This means that we, as a non-session group leader, can never regain a controlling terminal.
/// 4. chdir("/") to ensure that our process doesn't keep any directory in use. Failure to do this could make it so that an administrator couldn't unmount a filesystem, because it was our current directory. [Equivalently, we could change to any directory containing files important to the daemon's operation.]
/// 5. umask(0) so that we have complete control over the permissions of anything we write. We don't know what umask we may have inherited. [This step is optional]
/// 6. close() fds 0, 1, and 2. This releases the standard in, out, and error we inherited from our parent process. We have no way of knowing where these fds might have been redirected to. Note that many daemons use sysconf() to determine the limit _SC_OPEN_MAX. _SC_OPEN_MAX tells you the maximun open files/process. Then in a loop, the daemon can close all possible file descriptors. You have to decide if you need to do this or not. If you think that there might be file-descriptors open you should close them, since there's a limit on number of concurrent file descriptors.
/// 7. Establish new open descriptors for stdin, stdout and stderr. Even if you don't plan to use them, it is still a good idea to have them open. The precise handling of these is a matter of taste; if you have a logfile, for example, you might wish to open it as stdout or stderr, and open `/dev/null' as stdin; alternatively, you could open `/dev/console' as stderr and/or stdout, and `/dev/null' as stdin, or any other combination that makes sense for your particular daemon. 
///
/// https://linux.die.net/man/1/daemonize - interesting note about setuid-to-root executable and lockfile mechanism ineffective
/// https://linux.die.net/man/3/daemon - may not work as intended on Linux
/// daemon(3) implementation was taken from BSD and does not employ the double-fork technique
/// i.e. fork(2) -> setsid(2) -> fork(2)
/// which is necessary to ensure that the resulting daemon process is non-session group leader

Daemonize::Daemonize(const std::string& outFile, const std::string& errFile, const std::string& pidFile)
{
    std::string logFileDir = Utils::runningAsSudo() ? "/var/log/" : "/tmp/";
    std::string pidFileDir = Utils::runningAsSudo() ? "/var/run/" : "/tmp/";
    std::string outFilePath = logFileDir + outFile;
    std::string errFilePath = logFileDir + errFile;
    std::string pidFilePath = pidFileDir + pidFile;
    switch(fork())
    {
        case -1:
        {
            throw std::system_error(std::error_code(errno, std::system_category()));
        }
        case 0: break;
        default:
        {
            exit(EXIT_SUCCESS);
        }
    }

    if(setsid() == -1)
    {
        throw std::system_error(std::error_code(errno, std::system_category()));
    }

    signal(SIGCHLD, SIG_IGN);

    switch(fork())
    {
        case -1:
        {
            throw std::system_error(std::error_code(errno, std::system_category()));
        }
        case 0: break;
        default:
        {
            exit(EXIT_SUCCESS);
        }
    }

    umask(0);
    chdir("/");

    for(int fd = 0 ; fd < sysconf(_SC_OPEN_MAX) ; fd++)
        close(fd);
    
    stdin  = fopen("/dev/null", "r");
    if(stdin == NULL) // important for this to succeed as fd 0 will be assigned to stdout otherwise
        throw std::system_error(std::error_code(errno, std::system_category()));

    stdout = fopen(outFilePath.c_str(), "w+");
    if(stdout == NULL)
        throw std::system_error(std::error_code(errno, std::system_category()));

    stderr = fopen(errFilePath.c_str(), "w+");
    if(stderr == NULL)
        throw std::system_error(std::error_code(errno, std::system_category()));

    pidFd_ = open(pidFilePath.c_str(), O_RDWR|O_CREAT, 0640);
    if(pidFd_ == -1)
        throw std::system_error(std::error_code(errno, std::system_category()));

    if(lockf(pidFd_, F_TLOCK, 0) == -1)
        throw std::system_error(std::error_code(errno, std::system_category()));
    
    std::string pidString = std::to_string(getpid());
    write(pidFd_, pidString.c_str(), pidString.length());
}

Daemonize::~Daemonize()
{
    lockf(pidFd_, F_ULOCK, 0);
    close(pidFd_);
}

Daemonize& Daemonize::initiate(const std::string& outFile, const std::string& errFile, const std::string& pidFile)
{
    static Daemonize daemon(outFile, errFile, pidFile);
    return daemon;
}