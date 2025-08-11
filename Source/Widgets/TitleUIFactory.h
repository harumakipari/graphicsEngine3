#pragma once
#include "UIFactory.h"
#include "GameObject.h"
#include "Button.h"
#include "AudioSource.h"
#include "UIAnimationController.h"
#include "EasingComponent.h"

class TitleUIFactory
{
public:
	static void Create(Scene* scene)
	{
		//追加対象をセット
		UIFactory::SetObjectManager(&scene->objectManager);

		//フェード
		GameObject* fadeCanvas = UIFactory::CreateCanvas("FadeCanvas");
		GameObject* fadeImageObj = UIFactory::CreateImage("Fade", fadeCanvas);
		fadeCanvas->priority = 10;
		Image* fadeImage = fadeImageObj->GetComponent<Image>();
		fadeImageObj->rect->SetAnchorMin({ 0,0 });
		fadeImageObj->rect->SetAnchorMax({ 1,1 });
		fadeImageObj->rect->SetPivot({ 0,0 });
		fadeImage->color = { 0,0,0,0 };

		//FADE
		EasingComponent* easeFade = fadeCanvas->AddComponent<EasingComponent>(&fadeImage->color.a);
		EasingHandler handler;
		handler.SetEasing(EaseType::OutSine, 1.0f, 0.0f, 0.5f);
		handler.SetCompletedFunction([fadeCanvas]() {
			fadeCanvas->SetActive(false);
			}, true);
		easeFade->StartHandler(handler);

		//当たり判定隠す用Canvas
		GameObject* hideCanvas = UIFactory::CreateCanvas("HideCanvas"); 
		GameObject* hideImageObj = UIFactory::CreateImage("Hide", hideCanvas);
		hideImageObj->rect->SetAnchorMin({ 0,0 });
		hideImageObj->rect->SetAnchorMax({ 1,1 });
		hideImageObj->rect->SetPivot({ 0,0 });
		hideImageObj->GetComponent<Image>()->color = { 0,0,0,0 };
		hideCanvas->SetActive(false);
		hideCanvas->priority = 20;

		GameObject* titleCanvas = UIFactory::CreateCanvas("TitleCanvas");
	 	GameObject* titleButtonObj = UIFactory::CreateButton("TitleButton", titleCanvas, L"./Data/Textures/UI/start_button.png");
		AudioSource* buttonSe = titleButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");

		titleButtonObj->rect->SetAnchorMin({ 1,1 });
		titleButtonObj->rect->SetAnchorMax({ 1,1 });
		titleButtonObj->rect->SetPivot({ 1,1 });
		titleButtonObj->rect->size = { 528,297 };
		
		GameObject* selectCanvas = UIFactory::CreateCanvas("SelectCanvas");
		GameObject* selectGradation = UIFactory::CreateImage("SelectGradation", selectCanvas, L"./Data/Textures/UI/black_gradation.png");
		GameObject* selectFrame = UIFactory::CreateImage("SelectFrame", selectCanvas, L"./Data/Textures/UI/select_frame.png");
		
		GameObject* startButtonObj = UIFactory::CreateButton("Start", selectCanvas, L"./Data/Textures/UI/game_start.png");
		GameObject* tutorialButtonObj = UIFactory::CreateButton("Tutorial", selectCanvas, L"./Data/Textures/UI/tutorial.png");
		GameObject* backButtonObj = UIFactory::CreateButton("BackToTitle", selectCanvas, L"./Data/Textures/UI/back_to_title.png");
		AudioSource* startSe = startButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		AudioSource* tutorialSe = tutorialButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		AudioSource* backSe = backButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		
		selectGradation->rect->SetAnchorMin({ 0,0 });
		selectGradation->rect->SetAnchorMax({ 1,1 });
		selectGradation->rect->SetPivot({ 0,0 });

		selectFrame->rect->size = { 820, 775 };
		startButtonObj->rect->size = { 575, 125 };
		tutorialButtonObj->rect->size = { 575, 125 };
		backButtonObj->rect->size = { 575, 125 };

		selectFrame->rect->SetAnchorMin({ 1,0.5f });
		selectFrame->rect->SetAnchorMax({ 1,0.5f });
		selectFrame->rect->SetPivot({ 1,0.5f });
		selectFrame->rect->SetAnchoredPosition({ -100,0 });

		startButtonObj->rect->SetAnchorMin({ 1,0.5f });
		startButtonObj->rect->SetAnchorMax({ 1,0.5f });
		startButtonObj->rect->SetPivot({ 1,0.5f });
		startButtonObj->rect->SetAnchoredPosition({ -200,-200 });

		tutorialButtonObj->rect->SetAnchorMin({ 1,0.5f });
		tutorialButtonObj->rect->SetAnchorMax({ 1,0.5f });
		tutorialButtonObj->rect->SetPivot({ 1,0.5f });
		tutorialButtonObj->rect->SetAnchoredPosition({ -200,0 });

		backButtonObj->rect->SetAnchorMin({ 1,0.5f });
		backButtonObj->rect->SetAnchorMax({ 1,0.5f });
		backButtonObj->rect->SetPivot({ 1,0.5f });
		backButtonObj->rect->SetAnchoredPosition({ -200,200 });

		//最初は非アクティブ
		selectCanvas->SetActive(false);

		//タイトルボタン
		Button* button = titleButtonObj->GetComponent<Button>();
		titleButtonObj->rect->anchoredPosition.x = -50.0f;
		//選択画面ボタン
		Button* startButton = startButtonObj->GetComponent<Button>();
		Button* tutorialButton = tutorialButtonObj->GetComponent<Button>();
		Button* backButton = backButtonObj->GetComponent<Button>();

		//イージングコンポーネント
		EasingComponent* buttonEasing = titleButtonObj->AddComponent<EasingComponent>(&titleButtonObj->rect->anchoredPosition.x);
		EasingComponent* selectEasings[] = {
			selectGradation->AddComponent<EasingComponent>(&selectGradation->GetComponent<Image>()->color.a),
			selectFrame->AddComponent<EasingComponent>(&selectFrame->GetComponent<Image>()->color.a),
			startButtonObj->AddComponent<EasingComponent>(&startButtonObj->GetComponent<Image>()->color.a),
			tutorialButtonObj->AddComponent<EasingComponent>(&tutorialButtonObj->GetComponent<Image>()->color.a),
			backButtonObj->AddComponent<EasingComponent>(&backButtonObj->GetComponent<Image>()->color.a),
		};

		//Startボタン押したとき
		button->AddOnClickEvent([selectCanvas, buttonSe, buttonEasing, selectEasings, hideCanvas]() {
			//右に移動
			EasingHandler handler;
			handler.SetEasing(EaseType::OutQuart, -50.0f, 600.0f);
			buttonEasing->StartHandler(handler);
			
			//α値上げていく
			EasingHandler alpha;
			alpha.SetEasing(EaseType::Linear, 0.0f, 0.0f, 0.0f);
			alpha.SetWait(1.067f);
			alpha.SetEasing(EaseType::OutSine, 0.0f, 1.0f);
			alpha.SetCompletedFunction([hideCanvas]() {
				hideCanvas->SetActive(false);
				});
			for (EasingComponent* easing : selectEasings)
			{
				easing->StartHandler(alpha);
			}
			selectCanvas->SetActive(true);
			hideCanvas->SetActive(true);

			buttonSe->Play();
			});

		//ゲームスタートボタン
		startButton->AddOnClickEvent([startSe, easeFade, fadeCanvas]() {
			startSe->Play();
			fadeCanvas->SetActive(true);
			EasingHandler handler;
			handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.5f);
			handler.SetCompletedFunction([]() {
				const char* types[] = { "0", "1" };
				Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
				});
			easeFade->StartHandler(handler);
			});
		//チュートリアルボタン
		tutorialButton->AddOnClickEvent([tutorialSe, easeFade, fadeCanvas]() {
			tutorialSe->Play();
			fadeCanvas->SetActive(true);
			EasingHandler handler;
			handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 0.5f);
			handler.SetCompletedFunction([]() {
				const char* types[] = { "0", "1" };
				Scene::_transition("LoadingScene", { std::make_pair("preload", "TutorialScene"), std::make_pair("type", types[rand() % 2]) });
				});
			easeFade->StartHandler(handler);
			});


		//戻るボタン
		backButton->AddOnClickEvent([selectCanvas, backSe, buttonEasing, selectEasings, hideCanvas]() {
			//左に移動
			EasingHandler handler;
			handler.SetEasing(EaseType::OutQuart, 600.0f, -50.0f);
			buttonEasing->StartHandler(handler);

			//α値下げていく
			EasingHandler alpha;
			alpha.SetEasing(EaseType::OutSine, 1.0f, 0.0f);
			alpha.SetCompletedFunction([selectCanvas, hideCanvas]() {
				selectCanvas->SetActive(false);
				hideCanvas->SetActive(false);
				});
			for (EasingComponent* easing : selectEasings)
			{
				easing->StartHandler(alpha);
			}
			hideCanvas->SetActive(true);
			backSe->Play();
			});
	}
};