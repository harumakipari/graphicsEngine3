#include "InputModule.h"

#include "../../Engine/Input/InputSystem.h"

#include "EventSystem.h"

void InputModule::Process(float deltaTime) {
    //���͏����X�V
    pointerEventData->lastPosition.x = static_cast<float>(InputSystem::GetOldMousePositionX());
    pointerEventData->lastPosition.y = static_cast<float>(InputSystem::GetOldMousePositionY());
    pointerEventData->position = InputSystem::GetMousePosition();

    
    //���C�L���X�g����
    RaycastResult result = pointerEventData->pointerCurrentRaycast = eventSystem->RaycastAll();

    //�z�o�[���Ă���I�u�W�F�N�g���ς���Ă���
    if (pointerEventData->pointerEnter != result.gameObject) {
        //TODO: OnPointerExit�𔭍s
        ExecuteEvent<IPointerExitHandler>(pointerEventData->pointerEnter, pointerEventData, &IPointerExitHandler::Execute);

        pointerEventData->pointerEnter = result.gameObject;

        //TODO: OnPointerEnter�𔭍s
        ExecuteEvent<IPointerEnterHandler>(pointerEventData->pointerEnter, pointerEventData, &IPointerEnterHandler::Execute);
    }
    //�}�E�X�̏�Ԃ��Ƃ̏���
    {
        if (InputSystem::GetInputState("ok", InputStateMask::Trigger, DeviceFlags::MouseOnly)) {
            pointerEventData->pointerPressRaycast = result;
            //�I���I�u�W�F�N�g�������Ƃ��̍X�V����
            if (result.IsValid()) {
                pointerEventData->pointerPress = result.gameObject;
                //pointerEventData->pressPosition = result.gameObject->GetComponent<RectTransform>()->GetWorldPosition();
                pointerEventData->pressPosition = pointerEventData->position;
                pointerEventData->eligibleForClick = true;

                ExecuteEvent<IPointerDownHandler>(pointerEventData->pointerPress, pointerEventData, &IPointerDownHandler::Execute);
            }
            //�I���I�u�W�F�N�g�X�V
            if (result.gameObject != eventSystem->GetSelectedGameObject()) {
                ExecuteEvent<IDeselectHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IDeselectHandler::Execute);
                ExecuteEvent<ISelectHandler>(result.gameObject, pointerEventData, &ISelectHandler::Execute);
            }
            eventSystem->SetSelectedGameObject(result.gameObject);
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::None, DeviceFlags::MouseOnly)) {
            //�}�E�X�̈ړ��ʍX�V
            pointerEventData->delta.x = pointerEventData->position.x - pointerEventData->lastPosition.x;
            pointerEventData->delta.y = pointerEventData->position.y - pointerEventData->lastPosition.y;

            //�h���b�O�J�n����
            if (!pointerEventData->dragging) {
                //������Ă���̈ړ���
                float moveX = pointerEventData->position.x - pointerEventData->pressPosition.x;
                float moveY = pointerEventData->position.y - pointerEventData->pressPosition.y;

                pointerEventData->dragging = (abs(moveX) > dragThreshold || abs(moveY) > dragThreshold);

                //�h���b�O�J�n
                if (pointerEventData->dragging) {
                    pointerEventData->pointerDrag = result.gameObject;
                    ExecuteEvent<IBeginDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IBeginDragHandler::Execute);
                }
            }
            else {
                //�h���b�O��
                ExecuteEvent<IDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IDragHandler::Execute);
            }
        }
        else if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::MouseOnly)) {
            pointerEventData->lastPress = pointerEventData->pointerPress;
            pointerEventData->pointerPress = nullptr;

            //�h���b�O���I������Ƃ�
            if (pointerEventData->dragging) {
                ExecuteEvent<IEndDragHandler>(pointerEventData->pointerDrag, pointerEventData, &IEndDragHandler::Execute);
            }
            pointerEventData->dragging = false;
            pointerEventData->pointerDrag = nullptr;

            ExecuteEvent<IPointerUpHandler>(pointerEventData->lastPress, pointerEventData, &IPointerUpHandler::Execute);
            ExecuteEvent<IPointerClickHandler>(result.gameObject, pointerEventData, &IPointerClickHandler::Execute);
        }
    }

    //�L�[�{�[�h��Q�[���p�b�h��Submit����
    if (InputSystem::GetInputState("ok", InputStateMask::Release, DeviceFlags::KeyboardAndGamePad)) {
        ExecuteEvent<ISubmitHandler>(eventSystem->GetSelectedGameObject(), axisEventData, &ISubmitHandler::Execute);
    }

    //�I���I�u�W�F�N�g�ɑ΂���X�V����
    ExecuteEvent<IUpdateSelectedHandler>(eventSystem->GetSelectedGameObject(), pointerEventData, &IUpdateSelectedHandler::Execute);

    //Axis�C�x���g
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

    ////�z�C�[���ʍX�V
    //pointerEventData->scrollDelta = InputSystem::GetWheelDelta();

    //// �z�C�[���C�x���g
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