#include "EventSystem.h"
#include "../GraphicRaycaster.h"

EventSystem EventSystem::current;

void EventSystem::SetSelectedGameObject(GameObject* obj) {
	if (currentSelectedGameObject != obj) {
		activeModule->ExecuteEvent<IDeselectHandler>(currentSelectedGameObject, activeModule->GetEventData(), &IDeselectHandler::Execute);
		currentSelectedGameObject = obj;
		activeModule->ExecuteEvent<ISelectHandler>(currentSelectedGameObject, activeModule->GetEventData(), &ISelectHandler::Execute);
	}
}


RaycastResult EventSystem::RaycastAll() {
	RaycastResult result{};
	std::vector<RaycastResult> results{};
	
	//破棄処理
	raycasters.erase(std::remove_if(raycasters.begin(), raycasters.end(),
		[&](const std::weak_ptr<GraphicRaycaster>& raycaster) {
			return raycaster.expired();
		}),
		raycasters.end());
	erases.clear();

	//優先度でソート
	std::sort(raycasters.begin(), raycasters.end(),
		[](const std::weak_ptr<GraphicRaycaster>& a, const std::weak_ptr<GraphicRaycaster>& b) {
			return a.lock()->gameObject->priority < b.lock()->gameObject->priority;
		});
	
	//すべてのレイキャスターのレイキャスト処理
	for (auto& raycaster : /*GetCurrent()->*/raycasters) {
		if (raycaster.lock()->IsEnable()) {
			raycaster.lock()->Raycast(activeModule->GetEventData(), results);
		}
	}
	if (results.size() > 0) {
		result = results.back();
	}
	return result;
}

void EventSystem::DrawProperty()
{
	activeModule->DrawProperty();
}