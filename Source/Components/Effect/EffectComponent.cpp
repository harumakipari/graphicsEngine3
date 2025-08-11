#include "EffectComponent.h"

#include "Graphics/Core/Graphics.h"
#include "Core/Actor.h"
void EffectComponent::Initialize()
{
}

void EffectComponent::Tick(float deltaTime)
{
    switch (effectState_)
    {
    case EffectState::InActive:
        break;
    case EffectState::Initlaizeing:
        break;
    case EffectState::Active:
        if (effectDuration_ > 0.0f)
        {
            elapsedTime_ += deltaTime;
            if (elapsedTime_ >= effectDuration_)
            {
                effectState_ = EffectState::Ending;
            }
        }
        break;
    case EffectState::Ending:
        break;
    case EffectState::Finished:
        //effectState_ = EffectState::InActive;
        break;
    }
}


void EffectComponent::Activate()
{
    effectState_ = EffectState::Initlaizeing;
    //isActivated_ = true;
}

void EffectComponent::Initialized()
{
    effectState_ = EffectState::Active;
}

void EffectComponent::Deactivate()
{
    effectState_ = EffectState::Finished;
    //effectState_ = EffectState::InActive;

}


bool EffectComponent::IsPlay() const
{
    return effectState_ == EffectState::Initlaizeing;
}

