#ifndef SHOCK_WAVE_TARGET_REGISTRY_H
#define SHOCK_WAVE_TARGET_REGISTRY_H

#include <memory>
#include <unordered_set>
#include "Core/Actor.h"

class ShockWaveTargetRegistry
{
public:
    static void Register(std::shared_ptr<Actor> actor)
    {
        targets_.insert(actor);
    }

    static void Unregister(std::shared_ptr<Actor> actor)
    {
        targets_.erase(actor);
    }

    static const std::unordered_set<std::shared_ptr<Actor>>& GetTargets()
    {
        return targets_;
    }

private:
    static inline std::unordered_set<std::shared_ptr<Actor>> targets_;
};

#endif // !SHOCK_WAVE_TARGET_REGISTRY_H
