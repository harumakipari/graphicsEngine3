#pragma once

#include <vector>
#include <memory>
#include <string>
#include <Windows.h>
#include <DirectXMath.h>

#include "InputModule.h"

class GraphicRaycaster;
class GameObject;
struct RaycastResult;

class EventSystem
{
	static EventSystem current;
	static std::vector<EventSystem> eventSystems;
	
	GameObject* currentSelectedGameObject = nullptr;
	static inline std::unique_ptr<InputModule> activeModule;
	//std::vector<InputModule*> inputModules;
	//std::vector<InputModule*> eraseModules;
	static inline std::vector<std::weak_ptr<GraphicRaycaster>> raycasters;
	static inline std::vector<std::weak_ptr<GraphicRaycaster>> erases;
public:

	//これをフレームワークで呼び出す
	static void Initialize() {
		activeModule = std::make_unique<InputModule>(GetCurrent());
	}

	//これをフレームワークで呼び出す
	static void Update(float elapsedTime) {
		//破棄処理
		/*for (auto& raycaster : raycasters) {
			if (raycaster.expired()) {
				raycaster.reset();
			}
		}*/
		{
			raycasters.erase(std::remove_if(raycasters.begin(), raycasters.end(),
				[&](const std::weak_ptr<GraphicRaycaster>& raycaster) {
					return raycaster.expired();
				}),
				raycasters.end());
			erases.clear();
		}


		//アクティブな入力モジュールを更新
		activeModule->Process(elapsedTime);
	}

	static void DrawProperty();

	void SetSelectedGameObject(GameObject* obj);
	GameObject* GetSelectedGameObject() { return currentSelectedGameObject; }

	static void Reset()
	{
		current.currentSelectedGameObject = nullptr;
	}


	static EventSystem* GetCurrent() { return &current; }
	static RaycastResult RaycastAll();

	static void RegisterGraphicRaycaster(std::shared_ptr<GraphicRaycaster> raycaster) {
		/*GetCurrent()->*/raycasters.emplace_back(raycaster);
	}
	static void UnregisterGraphicRaycaster(std::shared_ptr<GraphicRaycaster> raycaster) {
		/*GetCurrent()->*/erases.emplace_back(raycaster);
	}

	//static void RegisterInputModule(InputModule* inputModule) {
	//	GetCurrent()->inputModules.emplace_back(inputModule);
	//}
	//static void UnregisterInputModule(InputModule* inputModule) {
	//	GetCurrent()->eraseModules.emplace_back(inputModule);
	//}
};