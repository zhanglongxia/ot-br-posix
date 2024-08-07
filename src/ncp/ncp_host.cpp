/*
 *  Copyright (c) 2024, The OpenThread Authors.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of the copyright holder nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#define OTBR_LOG_TAG "NCP_HOST"

#include "ncp_host.hpp"

#include <openthread/error.h>
#include <openthread/thread.h>

#include <openthread/openthread-system.h>

#include "lib/spinel/spinel_driver.hpp"

namespace otbr {
namespace Ncp {

NcpHost::NcpHost(const char *aInterfaceName, bool aDryRun)
    : mSpinelDriver(*static_cast<ot::Spinel::SpinelDriver *>(otSysGetSpinelDriver()))
{
    memset(&mConfig, 0, sizeof(mConfig));
    mConfig.mInterfaceName = aInterfaceName;
    mConfig.mDryRun        = aDryRun;
    mConfig.mSpeedUpFactor = 1;
}

const char *NcpHost::GetCoprocessorVersion(void)
{
    return mSpinelDriver.GetVersion();
}

void NcpHost::Init(void)
{
    otSysInit(&mConfig);
    mNcpSpinel.Init(mSpinelDriver);
}

void NcpHost::Deinit(void)
{
    mNcpSpinel.Deinit();
    otSysDeinit();
}

void NcpHost::GetDeviceRole(DeviceRoleHandler aHandler)
{
    mNcpSpinel.GetDeviceRole(aHandler);
}

void NcpHost::Process(const MainloopContext &aMainloop)
{
    mSpinelDriver.Process(&aMainloop);
}

void NcpHost::Update(MainloopContext &aMainloop)
{
    mSpinelDriver.GetSpinelInterface()->UpdateFdSet(&aMainloop);

    if (mSpinelDriver.HasPendingFrame())
    {
        aMainloop.mTimeout.tv_sec  = 0;
        aMainloop.mTimeout.tv_usec = 0;
    }
}

} // namespace Ncp
} // namespace otbr
