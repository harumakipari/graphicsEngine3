#include "LifeTimeComponent.h"

#include "Core/Actor.h"

void LifeTimeComponent::Tick(float deltaTime)
{
    if (!isStartCountDown_)
    {
        return;
    }

    elapsedTime_ += deltaTime;
    if (elapsedTime_ >= lifeTime_)
    {
        if (auto owner = owner_.lock())
        {
            //char buf[256];
            //sprintf_s(buf, "LifeTimeComponent::Tick Å® SetValid(false) åƒÇ—èoÇµÅBowner=%s\n", owner->GetName().c_str());
            //OutputDebugStringA(buf);
            owner->SetPendingDestroy();
            //owner->SetValid(false);
        }
        else
        {
            OutputDebugStringA("LifeTimeComponent::Tick Å® owner_.lock() é∏îsÅI\n");
        }
        isStartCountDown_ = false;
    }
}
