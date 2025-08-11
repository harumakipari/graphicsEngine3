#include "UIComponent.h"
#include "GameObject.h"

void UIComponent::SetEnable(bool set)
{
	if (gameObject != nullptr) {
		if (set) OnEnable();
		else OnDisable();
	}
	enable = set;
}

void UIComponent::Destroy() {
	gameObject->Destroy(this);
}