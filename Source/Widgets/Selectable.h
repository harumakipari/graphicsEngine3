#pragma once
#include "../Engine/Input/InputSystem.h"
#include "UIComponent.h"
#include "Image.h"
#include "Color.h"
#include "Events/EventHandlers.h"
#include "Events/EventSystem.h"
#include "ObjectManager.h"
#include "GameObject.h"

class Selectable : public UIComponent, public IPointerDownHandler, public IPointerUpHandler, 
	public IPointerEnterHandler, public IPointerExitHandler,
	public IMoveHandler, public ISelectHandler, public IDeselectHandler
{
public:
	struct Navigation {
		Selectable* up;
		Selectable* down;
		Selectable* left;
		Selectable* right;
	};
	Navigation navigation;
protected:
	friend class UIAnimationController;
	bool interactable = true;
	bool isHovered = false;
	bool isPressed = false;
	bool isSelected = false;
public:
	//bool isDragAccept = false;
	//bool isDragging = false;
	//XMFLOAT2 lastMousePos;
public:
	Image* image = nullptr;
	//Default : { 1,1,1,1 }
	Color defaultColor{ 1,1,1,1 };
	//Default : { 1.2f, 1.2f ,1.2f, 1.5f }
	Color selectedColor{ 1.2f, 1.2f, 1.2f, 1.5f };
	//Default : { 0.75f, 0.75f, 0.75f, 1.0f }
	Color hoveringColor{ 0.75f, 0.75f, 0.75f, 1.0f };
	//Default : { 0.5f, 0.5f, 0.5f, 1.0f }
	Color pressingColor{ 0.5f, 0.5f, 0.5f, 1.0f };

	Color disabledColor{ 0.4f, 0.4f, 0.4f, 1.0f };
public:
	Selectable() = default;
	virtual ~Selectable() override = default;

	virtual void OnPointerDown(PointerEventData* eventData) override {
		isPressed = true;
		UpdateVisual();
	}
	virtual void OnPointerUp(PointerEventData* eventData) override {
		isPressed = false;
		UpdateVisual();
	}
	virtual void OnPointerEnter(PointerEventData* eventData) override {
		isHovered = true;
		UpdateVisual();
	}
	virtual void OnPointerExit(PointerEventData* eventData) override {
		isHovered = false;
		UpdateVisual();
	}
	virtual void OnSelect(BaseEventData* eventData) override {
		isSelected = true;
		UpdateVisual();
	}
	virtual void OnDeselect(BaseEventData* eventData) override {
		isSelected = false;
		UpdateVisual();
	}
	virtual void OnMove(AxisEventData* eventData) override {

		Selectable* newSelectable = nullptr;

		switch (eventData->GetDirection())
		{
		case MoveDirection::Left:
			newSelectable = navigation.left;
			break;
		case MoveDirection::Right:
			newSelectable = navigation.right;
			break;
		case MoveDirection::Up:
			newSelectable = navigation.up;
			break;
		case MoveDirection::Down:
			newSelectable = navigation.down;
			break;
		default:
			break;
		}

		if (newSelectable != nullptr) {
			EventSystem::GetCurrent()->SetSelectedGameObject(newSelectable->gameObject);
		}
	}


	bool IsHovering() const { return isHovered; }

private:

	void UpdateVisual() {
		// 色の優先度：Pressing > Hovering > Selected > Default
		if (!interactable) image->color = disabledColor;
		else if (isPressed) image->color = pressingColor;
		else if (isHovered) image->color = hoveringColor;
		else if (isSelected) image->color = selectedColor;
		else image->color = defaultColor;
	}

public:
	bool IsInteractable() const { return interactable; }
	void SetInteractable(bool value) { interactable = value; UpdateVisual(); }

protected:
	virtual void DrawProperty() override {
#ifdef USE_IMGUI
		bool isInteractable = IsInteractable();
		if (ImGui::Checkbox("interactable", &isInteractable)) {
			SetInteractable(isInteractable);
		}

		//イベント
		{
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

			if (ImGui::TreeNodeEx("Navigation", ImGuiTreeNodeFlags_DefaultOpen)) {

				const char* directions[] = { "Up", "Down", "Left", "Right" };
				Selectable* navigations[] = { navigation.up, navigation.down, navigation.left, navigation.right };

				for (size_t i = 0; i < 4; i++) {
					ImGui::PushID(static_cast<int>(i));
					
					//ドロップ先
					ImGui::Text(directions[i]);
					//ImGui::SameLine();
					ImGui::Button(navigations[i] ? navigations[i]->gameObject->name.c_str() : "None(Selectable)");
					if (ImGui::BeginDragDropTarget()) {
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("GameObject")) {
							IM_ASSERT(payload->DataSize == sizeof(int*));
							int* pId = static_cast<int*>(payload->Data);

							if (GameObject* obj = ObjectManager::Find(*pId)) {
								navigations[i] = obj->GetComponent<Selectable>();
							}
						}
						ImGui::EndDragDropTarget();
					}

					ImGui::PopID();
				}

				navigation.up = navigations[0];
				navigation.down = navigations[1];
				navigation.left = navigations[2];
				navigation.right = navigations[3];

				ImGui::TreePop();
			}
			ImGui::PopStyleColor(3);
		}

		ImGui::ColorEdit4("DefaultColor", &defaultColor.r);
		ImGui::ColorEdit4("SelectedColor", &selectedColor.r);
		ImGui::ColorEdit4("HoveringColor", &hoveringColor.r);
		ImGui::ColorEdit4("PressingColor", &pressingColor.r);
		ImGui::ColorEdit4("DisabledColor", &disabledColor.r);
#endif // USE_IMGUI
	}
};