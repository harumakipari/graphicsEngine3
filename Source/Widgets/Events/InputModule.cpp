#include "InputModule.h"

#include "../../Engine/Input/InputSystem.h"

#include "EventSystem.h"

void InputModule::Process(float deltaTime) {
    //入力情報を更新
    pointerEventData->lastPosition.x = static_cast<float>(InputSystem::GetOldMousePositionX());
    pointerEventData->lastPosition.y = static_cast<float>(InputSystem::GetOldMousePositionY());
    pointerEventData->position = InputSystem::GetMousePosition();

    
    //レイキャスト処理
    RaycastResult result = pointerEventData->pointerCurrentRaycast = eventSystem->RaycastAll();

    //ホバーしているオブジェクトが変わってたら
    if (pointerEventData->pointerEnter != result.gameObject) {
        //TODO: OnPointerExitを発行
        ExecuteEvent<IPointerExitHandler>(pointerEventData->pointerEnter, pointerEventData, &IPointerExitHandler::Execute);

        pointerEventData->pointerEnter = result.gameObject;

        //TODO: OnPointerEnterを発行
        ExecuteEvent<IPointerEnterHandler>(pointerEventData->pointerEnter, pointerEventData, &IPointerEnterHandler::Execute);
    }
    //マウスの状態ごとの処理
    {
        if (InputSystem::GetInputState("ok", InputStateMask::Trigger, DeviceFlags::MouseOnly)) {
            pointerEventData->pointerPressRaycast = result;
            //選択オブジェクトがいたときの更新処理
            if (result.IsValid()) {
                pointerEventData->pointerPress = result.gameObject;
                //pointerEventData->pressPosition = result.gameObject->GetComponent<RectTransform>()->GetWorldPosition();
                pointerEventData->pressPosition = pointerEventData->position;
                pointerEventData->eligibleForClick = true;

                ExecuteEvent<IPointerDownHandler>(pointerEventData->pointerPress, pointerEventData, &IPointerDownHandler::Execute);
            }
            //選択オブジェクト更新
            if (result.gameObject != eventSystem->GetSelectedGameObject()) {
                ExecuteEvent<IDeselectHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IDeselectHandler::Execute);
                ExecuteEvent<ISelectHandler>(result.gameObject, pointerEventData, &ISelectHandler::Execute);
            }
            eventSystem->SetSelectedGameObject(result.gameObject);
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::None, DeviceFlags::MouseOnly)) {
            //マウスの移動量更新
            pointerEventData->delta.x = pointerEventData->position.x - pointerEventData->lastPosition.x;
            pointerEventData->delta.y = pointerEventData->position.y - pointerEventData->lastPosition.y;

            //ドラッグ開始判定
            if (!pointerEventData->dragging) {
                //押されてからの移動量
                float moveX = pointerEventData->position.x - pointerEventData->pressPosition.x;
                float moveY = pointerEventData->position.y - pointerEventData->pressPosition.y;

                pointerEventData->dragging = (abs(moveX) > dragThreshold || abs(moveY) > dragThreshold);

                //ドラッグ開始
                if (pointerEventData->dragging) {
                    pointerEventData->pointerDrag = result.gameObject;
                    ExecuteEvent<IBeginDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IBeginDragHandler::Execute);
                }
            }
            else {
                //ドラッグ中
                ExecuteEvent<IDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IDragHandler::Execute);
            }
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::MouseOnly)) {
            pointerEventData->lastPress = pointerEventData->pointerPress;
            pointerEventData->pointerPress = nullptr;

            //ドラッグが終わったとき
            if (pointerEventData->dragging) {
                ExecuteEvent<IEndDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IEndDragHandler::Execute);
            }
            pointerEventData->dragging = false;
            pointerEventData->pointerDrag = nullptr;

            ExecuteEvent<IPointerUpHandler>(pointerEventData->lastPress, pointerEventData, &IPointerUpHandler::Execute);
            ExecuteEvent<IPointerClickHandler>(result.gameObject, pointerEventData, &IPointerClickHandler::Execute);
        }
    }

    //キーボードやゲームパッドのSubmit判定
    if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::KeyboardAndGamePad)) {
        ExecuteEvent<ISubmitHandler>(eventSystem->GetSelectedGameObject(), axisEventData, &ISubmitHandler::Execute);
    }

    //選択オブジェクトに対する更新処理
    ExecuteEvent<IUpdateSelectedHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IUpdateSelectedHandler::Execute);

    //Axisイベント
    {
        lastMoveDir = axisEventData->moveDir;
        MoveDirection moveDir = axisEventData->moveDir = static_cast<MoveDirection>(static_cast<int>(InputSystem::GetAxisDirection()));

        if (moveDir != lastMoveDir) {
            moveCooldown = 0.0f;
            consecutiveMoveCount = 0;
        }

        if (moveDir != MoveDirection::None)
        {
            if (moveCooldown <= 0.0f)
            {
                ExecuteEvent<IMoveHandler>(eventSystem->GetSelectedGameObject(), axisEventData, &IMoveHandler::Execute);
                moveCooldown = (consecutiveMoveCount == 0) ? initialDelay : repeatInterval;
                consecutiveMoveCount++;
            }
        }
        else {
            moveCooldown = 0.0f;
            consecutiveMoveCount = 0;
        }

        if (moveCooldown > 0.0f) {
            moveCooldown -= deltaTime;
        }
    }

    ////ホイール量更新
    //pointerEventData->scrollDelta = InputSystem::GetWheelDelta();

    //// ホイールイベント
    //if (fabsf(pointerEventData->scrollDelta) > 0.01f) {
    //    ExecuteEvent<IScrollHandler>(result.gameObject, pointerEventData, &IScrollHandler::Execute);
    //}
}

void InputModule::DrawProperty() {
#ifdef USE_IMGUI

    ImGui::DragFloat("DragThreshold", &dragThreshold, 1.0f, 0.f, 1000.f);

    ImGui::Separator();

    ImGui::DragFloat2("ScreenPos", &pointerEventData->position.x);
    ImGui::DragFloat2("LastScreenPos", &pointerEventData->lastPosition.x);
    ImGui::DragFloat2("PressPos", &pointerEventData->pressPosition.x);
    ImGui::DragFloat2("Delta", &pointerEventData->delta.x);
    ImGui::Text("PointerEnter:%s", pointerEventData->pointerEnter ? pointerEventData->pointerEnter->name.c_str() : "None");
    ImGui::Text("PointerPress:%s", pointerEventData->pointerPress ? pointerEventData->pointerPress->name.c_str() : "None");
    ImGui::Text("LastPress:%s", pointerEventData->lastPress ? pointerEventData->lastPress->name.c_str() : "None");
    ImGui::Text("PointerDrag:%s", pointerEventData->pointerDrag ? pointerEventData->pointerDrag->name.c_str() : "None");
    bool dragging = pointerEventData->dragging;
    ImGui::Checkbox("Dragging", &dragging);
    ImGui::Separator();
    ImGui::Text("HitObject:%s", pointerEventData->pointerCurrentRaycast.gameObject ? pointerEventData->pointerCurrentRaycast.gameObject->name.c_str() : "None");
    ImGui::Text("CurrentSelectedGameObject:%s", eventSystem->GetSelectedGameObject() ? eventSystem->GetSelectedGameObject()->name.c_str() : "None");

    ImGui::InputFloat2("axis", &axisEventData->moveVector.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
    //ImGui::InputFloat2("axis", &axisEventData->moveVector.x, "%.3f", ImGuiInputTextFlags_ReadOnly);
    const char* dirs[] = { "Up", "Left", "Down", "Right", "None" };
    ImGui::Text("MoveDirection: %s", dirs[axisEventData->moveDir]);
    ImGui::Text("consecutiveMoveCount:%d", consecutiveMoveCount);
    ImGui::InputFloat("moveCooldown", &moveCooldown, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);

#endif // USE_IMGUI
}