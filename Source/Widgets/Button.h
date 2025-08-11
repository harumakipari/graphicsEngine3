#pragma once
#include "Selectable.h"
#include <functional>

class Button : public Selectable, public IPointerClickHandler, public ISubmitHandler
{
private:
	std::vector<std::function<void()>> onClickFunctions;

public:
	Button() = default;
	~Button() override = default;

	void Initialize() override {
		image = gameObject->GetComponent<Image>();
	}

	void OnPointerClick(PointerEventData* eventData) override {
		OnClick();
	}
	void OnSubmit(BaseEventData* eventData) override {
		OnClick();
	}

	void DrawProperty() override {
#ifdef USE_IMGUI
		Selectable::DrawProperty();
		ImGui::Checkbox("pressing", &isPressed);

#endif // USE_IMGUI
	}

	void AddOnClickEvent(std::function<void()> func) {
		onClickFunctions.emplace_back(func);
	}
private:
	void OnClick() {
		for (auto& function : onClickFunctions) 
		{
			function();
		}
	}
};