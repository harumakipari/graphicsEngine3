#ifndef ITEM_SPAWNER_COMPONENT_H
#define ITEM_SPAWNER_COMPONENT_H

#include <string>
#include <memory>

#include "Components/Base/SceneComponent.h"



class ItemSpawnerComponent :public SceneComponent
{
public:
    ItemSpawnerComponent(const std::string& name, std::shared_ptr<Actor> owner) :SceneComponent(name, owner)
    {
    }

    virtual ~ItemSpawnerComponent() {}

    void SpawnItems(int count/*, float beamPower = 1.0f*/, bool hasLifeTime = false, float itemLifeTimer = 8.0f);

private:

};


#endif // !ITEM_SPAWNER_COMPONENT_H
