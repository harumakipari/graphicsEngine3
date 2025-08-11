#include "UIFactory.h"
#include "GameObject.h"
#include "../Engine/Scene/Scene.h"
#include "ObjectManager.h"
#include "Canvas.h"
#include "GraphicRaycaster.h"
#include "Image.h"
#include "Button.h"
#include "Mask.h"
#include "Text.h"

GameObject* UIFactory::Create(const std::string& name) {
	std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
	object->Create(name);
	objectManager->Register(object);
	return object.get();
}

GameObject* UIFactory::CreateUI(const std::string& name, GameObject* canvas) {
	GameObject* obj = Create(name);
	obj->SetParent(ResolveCanvasObject(canvas));
	return obj;
}
GameObject* UIFactory::CreateCanvas(const std::string& name) {
	GameObject* obj = Create(name);
	obj->AddComponent<Canvas>();
	obj->AddComponent<GraphicRaycaster>();
	return obj;
}
//有効なポインタとして返す
GameObject* UIFactory::ResolveCanvasObject(GameObject* canvas) {
	if (!canvas) {
		if (GameObject* obj = objectManager->Find("Canvas")) {
			canvas = obj;
		}
		else {
			canvas = CreateCanvas("Canvas");
		}
	}
	return canvas;
}
GameObject* UIFactory::CreateImage(const std::string& name, GameObject* canvas, const wchar_t* sourceImage, bool addMask) {
	GameObject* obj = CreateUI(name, canvas);
	if (addMask)
	{
		obj->AddComponent<Mask>();
	}
	obj->AddComponent<Image>(Graphics::GetDevice(), sourceImage);
	return obj;
}
GameObject* UIFactory::CreateButton(const std::string& name, GameObject* canvas, const wchar_t* sourceImage, bool addMask) {
	GameObject* obj = CreateUI(name, canvas);
	if (addMask)
	{
		obj->AddComponent<Mask>();
	}
	obj->AddComponent<Image>(Graphics::GetDevice(), sourceImage);
	obj->AddComponent<Button>();
	return obj;
}
GameObject* UIFactory::CreateText(const std::string& name, GameObject* canvas, const std::string& fontFilePath) {
	GameObject* obj = CreateUI(name, canvas);
	obj->AddComponent<Text>(Graphics::GetDevice(), fontFilePath);
	return obj;
}
//GameObject* UIFactory::CreateToggle(const std::string& name, GameObject* canvas = nullptr, const wchar_t* background = L"./Data/Default/UISprite.png", const wchar_t* check = L"./Data/Default/check.png") {
//	GameObject* obj = CreateUI(name, canvas);
//	Toggle* toggle = obj->AddComponent<Toggle>();
//	GameObject* backGround = CreateImage("Background", canvas, background);
//	toggle->image = backGround->GetComponent<Image>();
//	toggle->image->rect->anchorMin = { 0,0 };
//	toggle->image->rect->anchorMax = { 1,1 };
//	backGround->SetParent(obj);
//
//	GameObject* checkObj = CreateImage("Checkmark", canvas, check);
//	checkObj->SetParent(backGround);
//	toggle->checkMark = checkObj->GetComponent<Image>();
//	toggle->checkMark->rect->anchorMin = { 0,0 };
//	toggle->checkMark->rect->anchorMax = { 1,1 };
//	toggle->checkMark->color = Color::Black;
//	return obj;
//}