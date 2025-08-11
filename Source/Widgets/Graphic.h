#pragma once
#include "UIComponent.h"
#include "Canvas.h"

class Graphic : public UIComponent
{
public:
	bool isRaycastTarget = true;

	Graphic() = default;
	virtual ~Graphic() override {
		if (Canvas* canvas = gameObject->GetComponentInParent<Canvas>())
			canvas->UnregisterGraphic(this);
	}

	void Awake() override {
		gameObject->GetComponentInParent<Canvas>()->RegisterGraphic(this);
	}

	bool Raycast(const XMFLOAT2& position) {
		if (isRaycastTarget) {
			return rect->Contains(position);
		}
		return false;
	}


	void DrawProperty() override {
#ifdef USE_IMGUI
		ImGui::Checkbox("RaycastTarget", &isRaycastTarget);
#endif // USE_IMGUI
	}
};