#pragma once
#include "UIComponent.h"
#include "Utils/EasingHandler.h"

class EasingComponent : public UIComponent
{
	EasingHandler handler;
	float* value = nullptr;
public:
	EasingComponent(float* value) : value(value) {};

	void SetValue(float* value) {
		this->value = value;
	}

	void StartHandler(const EasingHandler& handler) {
		this->handler = handler;
	}

	void Update(float deltaTime) override
	{
		if (value) {
			handler.Update(*value, deltaTime);
		}
	}
};

class TextEasingComponent : public UIComponent
{
	EasingHandler handler;
	std::wstring* value = nullptr;
public:
	TextEasingComponent(std::wstring* value) : value(value) {};

	void SetValue(std::wstring* value) {
		this->value = value;
	}

	void StartHandler(const EasingHandler& handler) {
		this->handler = handler;
	}

	void Update(float deltaTime) override
	{
		if (value && !handler.IsCompleted()) {
			float valueFloat = -1.0f;
			handler.Update(valueFloat, deltaTime);
			if (valueFloat > -0.5f)
			{
				*value = std::to_wstring(static_cast<int>(valueFloat));
			}
		}
	}
};