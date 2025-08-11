#pragma once
#include "UIFactory.h"
#include "GameObject.h"
#include "Button.h"
#include "Engine/Framework/Framework.h"
#include "Engine/Scene/Scene.h"

class PauseUIFactory
{
public:
	static void Create(Scene* scene, bool canvasSetActive = false)
	{
		//追加対象をセット
		UIFactory::SetObjectManager(&scene->objectManager);

		GameObject* menuCanvas = UIFactory::CreateCanvas("MenuCanvas");
		GameObject* menuButtonObj = UIFactory::CreateButton("MenuButton", menuCanvas, L"./Data/Textures/UI/menu.png");
		menuCanvas->SetActive(canvasSetActive);

		//左上基準
		menuButtonObj->rect->SetAnchorMin({ 0,0 });
		menuButtonObj->rect->SetAnchorMax({ 0,0 });
		menuButtonObj->rect->SetPivot({ 0,0 });
		menuButtonObj->rect->SetAnchoredPosition({ 50,50 });
		menuButtonObj->rect->size = { 100,100 };

		GameObject* pauseCanvas = UIFactory::CreateCanvas("PauseCanvas");
		GameObject* pausePanelObj = UIFactory::CreateImage("PausePanel", pauseCanvas, L"./Data/Textures/UI/pause_panel.png");
		GameObject* exitButtonObj = UIFactory::CreateButton("ExitButton", pauseCanvas, L"./Data/Textures/UI/exit.png");
		GameObject* closeButtonObj = UIFactory::CreateButton("CloseButton", pauseCanvas, L"./Data/Textures/UI/close.png");

		AudioSource* menuSe = menuButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		AudioSource* exitSe = exitButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		AudioSource* closeSe = closeButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");


		//左上基準
		closeButtonObj->rect->SetAnchorMin({ 0.5f,0.5f });
		closeButtonObj->rect->SetAnchorMax({ 0.5f,0.5f });
		closeButtonObj->rect->SetPivot({ 0.5f,0.5f });
		closeButtonObj->rect->SetAnchoredPosition({ 320,-100 });

		//exit
		exitButtonObj->rect->SetAnchoredPosition({ 0,60 });
		

		Button* menuButton = menuButtonObj->GetComponent<Button>();
		Button* exitButton = exitButtonObj->GetComponent<Button>();
		Button* closeButton = closeButtonObj->GetComponent<Button>();

		//メニューボタンが押されたら
		menuButton->AddOnClickEvent([menuCanvas, pauseCanvas, menuSe]() {
			menuCanvas->SetActive(false);
			pauseCanvas->SetActive(true);
			Framework::timeScale = 0.0f;
			menuSe->Play();
			});

		//閉じるボタンが押されたら
		closeButton->AddOnClickEvent([menuCanvas, pauseCanvas, closeSe]() {
			pauseCanvas->SetActive(false);
			menuCanvas->SetActive(true);
			Framework::timeScale = Framework::defaultTimeScale;
			closeSe->Play();
			});

		//Exitボタンが押されたら
		exitButton->AddOnClickEvent([exitSe]() {
			Framework::timeScale = Framework::defaultTimeScale;
			exitSe->Play();
			//フェードしたあとシーン遷移
			EasingHandler handler;
			handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.5f);
			handler.SetCompletedFunction([]() {
				const char* types[] = { "0", "1" };
				Scene::_transition("LoadingScene", { std::make_pair("preload", "BootScene"), std::make_pair("type", types[rand() % 2]) });
				});
			GameObject* fadeCanvas = ObjectManager::Find("FadeCanvas");
			fadeCanvas->SetActive(true);
			fadeCanvas->GetComponent<EasingComponent>()->StartHandler(handler);
			});

		pauseCanvas->SetActive(false);
	}
};