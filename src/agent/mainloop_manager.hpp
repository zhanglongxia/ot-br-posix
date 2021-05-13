
#ifndef OTBR_AGENT_MAINLOOP_MANAGER_HPP_
#define OTBR_AGENT_MAINLOOP_MANAGER_HPP_

#include <openthread-br/config.h>

#include <openthread/openthread-system.h>

#include <list>

#include "agent/ncp_openthread.hpp"
#include "common/mainloop.hpp"

namespace otbr {

class MainloopManager
{
public:
    typedef MainloopProcessor *(*MainloopProcessorCallback)(otbr::Ncp::ControllerOpenThread *aNcp);

    MainloopManager(MainloopProcessorCallback *aFunctions, uint8_t aNumCallbacks)
        : mCallbacks(aFunctions)
        , mNumCallbacks(aNumCallbacks)
        , mList()
    {
    }

    void Init(otbr::Ncp::ControllerOpenThread &aNcp)
    {
        MainloopProcessor *processor;

        for (int i = 0; i < mNumCallbacks; i++)
        {
            processor = mCallbacks[i](&aNcp);
            Add(processor);
            processor->Init();
        }
    }

    void Add(MainloopProcessor *aProcessor) { mList.emplace_back(aProcessor); }

    void Update(MainloopContext &aMainloop)
    {
        for (std::list<MainloopProcessor *>::iterator it = mList.begin(); it != mList.end(); ++it)
        {
            (*it)->Update(aMainloop);
        }
    }

    void Process(const MainloopContext &aMainloop)
    {
        for (std::list<MainloopProcessor *>::iterator it = mList.begin(); it != mList.end(); ++it)
        {
            (*it)->Process(aMainloop);
        }
    }

private:
    MainloopProcessorCallback *    mCallbacks;
    uint8_t                        mNumCallbacks;
    std::list<MainloopProcessor *> mList;
};
} // namespace otbr
#endif // OTBR_COMMON_MAINLOOP_MANAGER_HPP_
