#include "PhysicsUtility.h"

#include "Physics/CollisionSystem.h"


void PhysicsTest::DebugShapePosition()
{
#ifdef USE_IMGUI

    for (auto& pair : CollisionSystem::allComponents_) // —á: “o˜^‚³‚ê‚½CollisionComponent‚ÌƒŠƒXƒg
    {
        //auto actor = pair.first;
        //auto shape = pair.second;
        //const Transform& transform = shape.lock()->GetComponentWorldTransform();
        //const DirectX::XMFLOAT3& pos = transform.GetLocation();

        //std::string label = actor ? actor->GetName() : "Unknown Actor";
        //ImGui::Text("%s: (%.2f, %.2f, %.2f)", label.c_str(), pos.x, pos.y, pos.z);
    }
#endif
}

void PhysicsTest::Finalize()
{
    CollisionSystem::ClearAll();
}

