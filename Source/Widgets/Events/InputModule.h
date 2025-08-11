#pragma once


#include <memory>
#include <functional>

class GameObject;

#include <DirectXMath.h>
#include "../GameObject.h"
#include "PointerEventData.h"
#include "AxisEventData.h"
#include "EventHandlers.h"

class EventSystem;

class InputModule
{
    EventSystem* eventSystem = nullptr;
    float dragThreshold = 2.f;

    float moveCooldown = 0.0f;
    int consecutiveMoveCount = 0;
    
    float initialDelay = 0.4f;
    float repeatInterval = 0.1f;
    MoveDirection lastMoveDir = MoveDirection::None;

    std::shared_ptr<PointerEventData> pointerEventData;
    std::shared_ptr<AxisEventData> axisEventData;
public:
    InputModule(EventSystem* eventSystem) : eventSystem(eventSystem){
        pointerEventData = std::make_shared<PointerEventData>(eventSystem);
        axisEventData = std::make_shared<AxisEventData>(eventSystem);
    }


    std::shared_ptr<PointerEventData> GetEventData() {
        return pointerEventData;
    }

    void Process(float deltaTime);

    template<class T>
    void ExecuteEvent(GameObject* target, std::shared_ptr<BaseEventData> eventData, std::function<void(T*, BaseEventData*)> callback) {
        if (target) {
            bool found = false;
            for (auto& component : target->GetAllComponents()) {
                T* handler = dynamic_cast<T*>(component.get());
                if (handler) {
                    callback(handler, eventData.get());
                    found = true;
                }
            }
            //イベントハンドラが見つからなかったら、親を探索
            if (!found) {
                ExecuteEvent<T>(target->parent, eventData, callback);
            }
        }
    }

    DirectX::XMFLOAT2 GetPointerPosition() const {
        return pointerEventData->position;
    }

    void DrawProperty();
};