#pragma once
#include "UIComponent.h"

#include "Graphic.h"

#include "Events/EventSystem.h"
#include "Canvas.h"

#include "Events/PointerEventData.h"
#include "Events/RaycastResult.h"

class GraphicRaycaster : public UIComponent
{
public:
	void OnEnable() override {
		EventSystem::RegisterGraphicRaycaster(gameObject->GetComponentShared<GraphicRaycaster>());
	}

	void OnDisable() override {
		EventSystem::UnregisterGraphicRaycaster(gameObject->GetComponentShared<GraphicRaycaster>());
	}
	
	void Raycast(std::shared_ptr<PointerEventData> eventData, std::vector<RaycastResult>& resultAppendList) {
		if (Canvas* canvas = gameObject->GetComponent<Canvas>()) {

			for (Graphic* graphic : canvas->GetGraphics()) {
				if (!graphic->IsEnable()) continue;
				if (!graphic->Raycast(eventData->position)) continue;

				//ƒqƒbƒg‚µ‚½‚ç’Ç‰Á
				RaycastResult result;
				result.gameObject = graphic->gameObject;
				result.module = this;
				result.screenPosition = eventData->position;

				resultAppendList.push_back(result);
			}
		}
	}

};