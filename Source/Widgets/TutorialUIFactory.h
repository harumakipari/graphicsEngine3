#pragma once
#include "UIFactory.h"
#include "GameObject.h"
#include "Text.h"
#include "Counter.h"

class TutorialUIFactory
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

		GameObject* tutorialCanvas = UIFactory::CreateCanvas("TutorialCanvas");
		tutorialCanvas->rect->SetAnchoredPosition({ 0,50 });

		GameObject* elements[11]{};

		GameObject* start = UIFactory::CreateImage("TutorialStart", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_start.png");
		elements[0] = UIFactory::CreateImage("TutorialMove", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_1.png");
		elements[1] = UIFactory::CreateImage("TutorialCollect", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_2.png");
		elements[2] = UIFactory::CreateImage("TutorialFirstAttack", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_3.png");
		elements[3] = UIFactory::CreateImage("TutorialManyCollect", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_4.png");
		elements[4] = UIFactory::CreateImage("TutorialInfo1", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_!1.png");
		elements[5] = UIFactory::CreateImage("TutorialSecondAttack", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_5.png");
		elements[6] = UIFactory::CreateImage("TutorialInfo2", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_!2.png");
		elements[7] = UIFactory::CreateImage("TutorialAim", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_6.png");
		elements[8] = UIFactory::CreateImage("TutorialInfo3", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_!3.png");
		elements[9] = UIFactory::CreateImage("TutorialBreakAttack", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_7.png");
		elements[10] = UIFactory::CreateImage("TutorialInfo4", tutorialCanvas, L"./Data/Textures/UI/Tutorial/tutorial_!4.png");
		GameObject* battleStart = UIFactory::CreateImage("BattleStart", tutorialCanvas, L"./Data/Textures/UI/Tutorial/battle_start.png");
		battleStart->SetActive(false);

		/*GameObject* checkFrame = UIFactory::CreateImage("CheckFrame", tutorialCanvas, L"./Data/Textures/UI/Tutorial/check_frame.png");
		GameObject* check = UIFactory::CreateImage("Check", tutorialCanvas, L"./Data/Textures/UI/Tutorial/check.png");
		checkFrame->SetActive(false);
		check->SetActive(false);

		checkFrame->rect->SetAnchorMin({ 1,0 });
		checkFrame->rect->SetAnchorMax({ 1,0 });
		checkFrame->rect->SetPivot({ 1,0 });
		checkFrame->rect->SetAnchoredPosition({ 0,0 });
		checkFrame->rect->size = { 638,359 };

		check->rect->SetAnchorMin({ 1,0 });
		check->rect->SetAnchorMax({ 1,0 });
		check->rect->SetPivot({ 1,0 });
		check->rect->SetAnchoredPosition({ 0,0 });
		check->rect->size = { 638,359 };*/


		//進捗テキスト（数字）
		//GameObject* testCanvas = UIFactory::CreateCanvas("TestCanvas");
		{
			//数字
			GameObject* tutorialNumber = UIFactory::CreateText("TutorialNum3", tutorialCanvas, "./Data/Font/tutorial_number.fnt");
			tutorialNumber->rect->SetAnchorMin({ 1,0 });
			tutorialNumber->rect->SetAnchorMax({ 1,0 });
			tutorialNumber->rect->SetAnchoredPosition({ -210,280 });
			Text* text = tutorialNumber->GetComponent<Text>();
			text->text = L"0/20";
			text->alignRight = true;
			text->fontSize = 36.0f;
			text->color = { 0,1,1,1 };

			Counter* counter = tutorialNumber->AddComponent<Counter>();
			counter->SetListener(TutorialStep::ManyCollect);
			counter->SetText(text, 20);
			tutorialNumber->SetActive(false);
		}
		//進捗テキスト（数字）
		{
			//数字
			GameObject* tutorialNumber = UIFactory::CreateText("TutorialNum9", tutorialCanvas, "./Data/Font/tutorial_number.fnt");
			tutorialNumber->rect->SetAnchorMin({ 1,0 });
			tutorialNumber->rect->SetAnchorMax({ 1,0 });
			tutorialNumber->rect->SetAnchoredPosition({ -210,280 });
			Text* text = tutorialNumber->GetComponent<Text>();
			text->text = L"0/5";
			text->alignRight = true;
			text->fontSize = 36.0f;
			text->color = { 0,1,1,1 };

			Counter* counter = tutorialNumber->AddComponent<Counter>();
			counter->SetListener(TutorialStep::ManyCollect2);
			counter->SetText(text, 5);
			tutorialNumber->SetActive(false);
		}

		//スタートは左上基準
		start->rect->SetAnchorMin({ 0,0 });
		start->rect->SetAnchorMax({ 0,0 });
		start->rect->SetPivot({ 0,0 });
		start->rect->SetAnchoredPosition({ 0,0 });

		//BattleStartは左上基準
		battleStart->rect->SetAnchorMin({ 0,0 });
		battleStart->rect->SetAnchorMax({ 0,0 });
		battleStart->rect->SetPivot({ 0,0 });
		battleStart->rect->SetAnchoredPosition({ 0,0 });

		for (GameObject* element : elements)
		{
			//右上基準
			element->rect->SetAnchorMin({ 1,0 });
			element->rect->SetAnchorMax({ 1,0 });
			element->rect->SetPivot({ 1,0 });
			element->rect->SetAnchoredPosition({ 0,0 });
			element->SetActive(false);
		}
	}
};