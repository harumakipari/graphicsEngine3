#pragma once
#include <memory>
#include "../Engine/Scene/Scene.h"
class GameObject;

class UIFactory
{
	static inline ObjectManager* objectManager = nullptr;
public:
	static void SetObjectManager(ObjectManager* manager) { objectManager = manager; }

	static GameObject* Create(const std::string& name);

	static GameObject* CreateUI(const std::string& name, GameObject* canvas = nullptr);
	static GameObject* CreateCanvas(const std::string& name);
	//有効なポインタとして返す
	static GameObject* ResolveCanvasObject(GameObject* canvas);
	static GameObject* CreateImage(const std::string& name, GameObject* canvas = nullptr, const wchar_t* sourceImage = nullptr, bool addMask = false);
	static GameObject* CreateButton(const std::string& name, GameObject* canvas = nullptr, const wchar_t* sourceImage = nullptr, bool addMask = false);
	static GameObject* CreateText(const std::string& name, GameObject* canvas = nullptr, const std::string& fontFilePath = "");
	//GameObject* CreateToggle(const std::string& name, GameObject* canvas = nullptr, const wchar_t* background = L"./Data/Default/UISprite.png", const wchar_t* check = L"./Data/Default/check.png");
};