/*
 *    Copyright (c) 2017, The OpenThread Authors.
 *    All rights reserved.
 *
 *    Redistribution and use in source and binary forms, with or without
 *    modification, are permitted provided that the following conditions are met:
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *    3. Neither the name of the copyright holder nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *    POSSIBILITY OF SUCH DAMAGE.
 */

#include <openthread-br/config.h>

#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <assert.h>
#include <openthread/logging.h>
#include <openthread/platform/radio.h>

#include "agent/agent_instance.hpp"
#include "common/code_utils.hpp"
#include "common/logging.hpp"
#include "common/mainloop.hpp"
#include "common/mainloop_manager.hpp"
#include "common/types.hpp"
#include "ncp/ncp_openthread.hpp"
#if OTBR_ENABLE_REST_SERVER
#include "rest/rest_web_server.hpp"
using otbr::rest::RestWebServer;
#endif
#if OTBR_ENABLE_DBUS_SERVER
#include "dbus/server/dbus_agent.hpp"
using otbr::DBus::DBusAgent;
#endif
#if OTBR_ENABLE_OPENWRT
#include "openwrt/ubus/otubus.hpp"
using otbr::ubus::UBusAgent;
#endif
#if OTBR_ENABLE_VENDOR_SERVER
#include "agent/vendor.hpp"
using otbr::vendor::VendorServer;
#endif

static const char kSyslogIdent[]          = "otbr-agent";
static const char kDefaultInterfaceName[] = "wpan0";

enum
{
    OTBR_OPT_BACKBONE_INTERFACE_NAME = 'B',
    OTBR_OPT_DEBUG_LEVEL             = 'd',
    OTBR_OPT_HELP                    = 'h',
    OTBR_OPT_INTERFACE_NAME          = 'I',
    OTBR_OPT_VERBOSE                 = 'v',
    OTBR_OPT_VERSION                 = 'V',
    OTBR_OPT_SHORTMAX                = 128,
    OTBR_OPT_RADIO_VERSION,
};

using otbr::MainloopManager;

static jmp_buf sResetJump;
static bool    sShouldTerminate = false;

void __gcov_flush();

// Default poll timeout.
static const struct timeval kPollTimeout = {10, 0};
static const struct option  kOptions[]   = {
    {"backbone-ifname", required_argument, nullptr, OTBR_OPT_BACKBONE_INTERFACE_NAME},
    {"debug-level", required_argument, nullptr, OTBR_OPT_DEBUG_LEVEL},
    {"help", no_argument, nullptr, OTBR_OPT_HELP},
    {"thread-ifname", required_argument, nullptr, OTBR_OPT_INTERFACE_NAME},
    {"verbose", no_argument, nullptr, OTBR_OPT_VERBOSE},
    {"version", no_argument, nullptr, OTBR_OPT_VERSION},
    {"radio-version", no_argument, nullptr, OTBR_OPT_RADIO_VERSION},
    {0, 0, 0, 0}};

static void HandleSignal(int aSignal)
{
    sShouldTerminate = true;
    signal(aSignal, SIG_DFL);
}

static int Mainloop(otbr::AgentInstance &aInstance)
{
    int error = EXIT_SUCCESS;

#if OTBR_ENABLE_OPENWRT
    UBusAgent ubusAgent{aInstance.GetNcp()};
    ubusAgent.Init();
#endif

#if OTBR_ENABLE_REST_SERVER
    RestWebServer restWebServer{aInstance.GetNcp()};
    restWebServer.Init();
#endif

#if OTBR_ENABLE_DBUS_SERVER
    DBusAgent dbusAgent{aInstance.GetNcp()};
    dbusAgent.Init();
#endif

#if OTBR_ENABLE_VENDOR_SERVER
    VendorServer vendorServer{aInstance.GetNcp()};
    vendorServer.Init();
#endif

    otbrLogInfo("Border router agent started.");
    // allow quitting elegantly
    signal(SIGTERM, HandleSignal);

    while (!sShouldTerminate)
    {
        otbr::MainloopContext mainloop;
        int                   rval;

        mainloop.mMaxFd   = -1;
        mainloop.mTimeout = kPollTimeout;

        FD_ZERO(&mainloop.mReadFdSet);
        FD_ZERO(&mainloop.mWriteFdSet);
        FD_ZERO(&mainloop.mErrorFdSet);

        MainloopManager::GetInstance().Update(mainloop);

        rval = select(mainloop.mMaxFd + 1, &mainloop.mReadFdSet, &mainloop.mWriteFdSet, &mainloop.mErrorFdSet,
                      &mainloop.mTimeout);

        if (rval >= 0)
        {
            MainloopManager::GetInstance().Process(mainloop);
        }
        else if (errno != EINTR)
        {
            error = OTBR_ERROR_ERRNO;
            otbrLogErr("select() failed: %s", strerror(errno));
            break;
        }
    }

    return error;
}

static void PrintHelp(const char *aProgramName)
{
    fprintf(stderr, "Usage: %s [-I interfaceName] [-B backboneIfName] [-d DEBUG_LEVEL] [-v] RADIO_URL [RADIO_URL]\n",
            aProgramName);
    fprintf(stderr, "%s", otSysGetRadioUrlHelpString());
}

static void PrintVersion(void)
{
    printf("%s\n", OTBR_PACKAGE_VERSION);
}

static void PrintRadioVersion(otInstance *aInstance)
{
    printf("%s\n", otPlatRadioGetVersionString(aInstance));
}

static void OnAllocateFailed(void)
{
    otbrLogCrit("Allocate failure, exiting...");
    exit(1);
}

static int realmain(int argc, char *argv[])
{
    otbrLogLevel              logLevel = OTBR_LOG_INFO;
    int                       opt;
    int                       ret                   = EXIT_SUCCESS;
    const char *              interfaceName         = kDefaultInterfaceName;
    const char *              backboneInterfaceName = "";
    bool                      verbose               = false;
    bool                      printRadioVersion     = false;
    std::vector<const char *> radioUrls;

    std::set_new_handler(OnAllocateFailed);

    while ((opt = getopt_long(argc, argv, "B:d:hI:Vv", kOptions, nullptr)) != -1)
    {
        switch (opt)
        {
        case OTBR_OPT_BACKBONE_INTERFACE_NAME:
            backboneInterfaceName = optarg;
            break;

        case OTBR_OPT_DEBUG_LEVEL:
            logLevel = static_cast<otbrLogLevel>(atoi(optarg));
            VerifyOrExit(logLevel >= OTBR_LOG_EMERG && logLevel <= OTBR_LOG_DEBUG, ret = EXIT_FAILURE);
            break;

        case OTBR_OPT_INTERFACE_NAME:
            interfaceName = optarg;
            break;

        case OTBR_OPT_VERBOSE:
            verbose = true;
            break;

        case OTBR_OPT_VERSION:
            PrintVersion();
            ExitNow();
            break;

        case OTBR_OPT_HELP:
            PrintHelp(argv[0]);
            ExitNow(ret = EXIT_SUCCESS);
            break;

        case OTBR_OPT_RADIO_VERSION:
            printRadioVersion = true;
            break;

        default:
            PrintHelp(argv[0]);
            ExitNow(ret = EXIT_FAILURE);
            break;
        }
    }

    otbrLogInit(kSyslogIdent, logLevel, verbose);
    otbrLogInfo("Running %s", OTBR_PACKAGE_VERSION);
    otbrLogInfo("Thread version: %s", otbr::Ncp::ControllerOpenThread::GetThreadVersion());
    otbrLogInfo("Thread interface: %s", interfaceName);
    otbrLogInfo("Backbone interface: %s", backboneInterfaceName);

    for (int i = optind; i < argc; i++)
    {
        otbrLogInfo("Radio URL: %s", argv[i]);
        radioUrls.push_back(argv[i]);
    }

    {
        otbr::Ncp::ControllerOpenThread ncpOpenThread{interfaceName, radioUrls, backboneInterfaceName};
        otbr::AgentInstance             instance(ncpOpenThread);

        otbr::InstanceParams::Get().SetThreadIfName(interfaceName);
        otbr::InstanceParams::Get().SetBackboneIfName(backboneInterfaceName);

        SuccessOrExit(ret = instance.Init());

        if (printRadioVersion)
        {
            PrintRadioVersion(ncpOpenThread.GetInstance());
            ExitNow(ret = EXIT_SUCCESS);
        }

        SuccessOrExit(ret = Mainloop(instance));
    }

    otbrLogDeinit();

exit:
    return ret;
}

void otPlatReset(otInstance *aInstance)
{
    OT_UNUSED_VARIABLE(aInstance);

    gPlatResetReason = OT_PLAT_RESET_REASON_SOFTWARE;

    otSysDeinit();

    longjmp(sResetJump, 1);
    assert(false);
}

int main(int argc, char *argv[])
{
    if (setjmp(sResetJump))
    {
        alarm(0);
#if OPENTHREAD_ENABLE_COVERAGE
        __gcov_flush();
#endif
        execvp(argv[0], argv);
    }

    return realmain(argc, argv);
}
