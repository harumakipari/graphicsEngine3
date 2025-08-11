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
            //sprintf_s(buf, "LifeTimeComponent::Tick �� SetValid(false) �Ăяo���Bowner=%s\n", owner->GetName().c_str());
            //OutputDebugStringA(buf);
            owner->SetPendingDestroy();
            //owner->SetValid(false);
        }
        else
        {
            OutputDebugStringA("LifeTimeComponent::Tick �� owner_.lock() ���s�I\n");
        }
        isStartCountDown_ = false;
    }
}
