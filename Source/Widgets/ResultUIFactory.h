#pragma once
#include "UIFactory.h"
#include "GameObject.h"
#include "Text.h"
#include "Game/Managers/GameManager.h"
#include "Engine/Scene/Scene.h"
#include "EventSequence.h"
#include "EasingComponent.h"
#include "Timer.h"

#define SCORE_S 4
#define SCORE_A 3
#define SCORE_B 2
#define SCORE_C 1

class ResultUIFactory
{
public:
	static void Create(Scene* scene, bool isCleared)
	{
		//追加対象をセット
		UIFactory::SetObjectManager(&scene->objectManager);

		//フェード
		GameObject* fadeCanvas = UIFactory::CreateCanvas("ResultFadeCanvas");
		GameObject* fadeImageObj = UIFactory::CreateImage("ResultFade", fadeCanvas);
		fadeCanvas->priority = 20;
		Image* fadeImage = fadeImageObj->GetComponent<Image>();
		fadeImageObj->rect->SetAnchorMin({ 0,0 });
		fadeImageObj->rect->SetAnchorMax({ 1,1 });
		fadeImageObj->rect->SetPivot({ 0,0 });
		fadeImage->color = { 0,0,0,1 };
		//FADE
		EasingComponent* easeFade = fadeCanvas->AddComponent<EasingComponent>(&fadeImage->color.a);

		//Canvas
		GameObject* resultCanvas = UIFactory::CreateCanvas("ResultCanvas");
		GameObject* resultFrame = UIFactory::CreateImage("ResultFrame", resultCanvas, L"./Data/Textures/UI/Result/result.png");

		//数字
		int attackCount = GameManager::GetBossDamageCount();
		int buildBrokeCount = GameManager::GetBuildBrokeCount();
		int playerDamageCount = GameManager::GetPlayerDamageCount();

		GameObject* texts[3]{};
		texts[0] = UIFactory::CreateText("AttackCount", resultCanvas, "./Data/Font/number.fnt");
		texts[1] = UIFactory::CreateText("BuildBrokeCount", resultCanvas, "./Data/Font/number.fnt");
		texts[2] = UIFactory::CreateText("PlayerDamageCount", resultCanvas, "./Data/Font/number.fnt");

		Text* textComponent[3]{};
		int values[3] = { attackCount, buildBrokeCount, playerDamageCount };
		
		for (int i = 0; i < 3; i++)
		{
			textComponent[i] = texts[i]->GetComponent<Text>();
			textComponent[i]->alignRight = true;
			TextEasingComponent* easeText = texts[i]->AddComponent<TextEasingComponent>(&textComponent[i]->text);
			EasingHandler handler;
			handler.SetWait(static_cast<float>((i * 0.5f) + 0.5f));
			handler.SetEasing(EaseType::Linear, 0.0f, static_cast<float>(values[i]), 0.5f);
			easeText->StartHandler(handler);
		}
		textComponent[0]->rect->SetAnchoredPosition({ 100, 50 });
		textComponent[1]->rect->SetAnchoredPosition({ 100, 205 });
		textComponent[2]->rect->SetAnchoredPosition({ 100, 360 });

		//textComponent[0]->text = std::to_wstring(attackCount);
		//textComponent[1]->text = std::to_wstring(buildBrokeCount);
		//textComponent[2]->text = std::to_wstring(playerDamageCount);
		//textComponent[0]->text = std::to_wstring(0);
		//textComponent[1]->text = std::to_wstring(0);
		//textComponent[2]->text = std::to_wstring(0);
		textComponent[0]->text = L"";
		textComponent[1]->text = L"";
		textComponent[2]->text = L"";

		//texts[0]->rect->SetAnchoredPosition({})

		//ランク
		const wchar_t* filePath[5] = {
			L"./Data/Textures/UI/Result/rank_F.png",
			L"./Data/Textures/UI/Result/rank_C.png",
			L"./Data/Textures/UI/Result/rank_B.png",
			L"./Data/Textures/UI/Result/rank_A.png",
			L"./Data/Textures/UI/Result/rank_S.png",
		};
		
		int time = ObjectManager::Find("TimerText")->GetComponent<Timer>()->GetTime();

		int index = 0;
		int score = 0;
		//クリア時のみランク計算
		if (isCleared) 
		{
			//タイマー
			{
				if (time > 210)
				{//C
					score += SCORE_C;
				}
				else if (time > 150)
				{//B
					score += SCORE_B;
				}
				else if (time > 90)
				{//A
					score += SCORE_A;
				}
				else
				{//S
					score += SCORE_S;
				}
			}
			//攻撃回数
			{
				if (attackCount > 20)
				{//C
					score += SCORE_C;
				}
				else if (attackCount > 15)
				{//B
					score += SCORE_B;
				}
				else if (attackCount > 10)
				{//A
					score += SCORE_A;
				}
				else
				{//S
					score += SCORE_S;
				}
			}
			//ビル破壊数
			{
				if (buildBrokeCount > 45)
				{//C
					score += SCORE_C;
				}
				else if (attackCount > 35)
				{//B
					score += SCORE_B;
				}
				else if (attackCount > 25)
				{//A
					score += SCORE_A;
				}
				else
				{//S
					score += SCORE_S;
				}
			}
			//被弾回数
			{
				if (playerDamageCount > 5)
				{//C
					score += SCORE_C;
				}
				else if (playerDamageCount > 3)
				{//B
					score += SCORE_B;
				}
				else if (playerDamageCount > 0)
				{//A
					score += SCORE_A;
				}
				else
				{//S
					score += SCORE_S;
				}
			}

			//ランク
			{
				if (score < 7)
				{
					index = SCORE_C;
				}
				else if (score < 12)
				{
					index = SCORE_B;
				}
				else if (score < 15)
				{
					index = SCORE_A;
				}
				else
				{
					index = SCORE_S;
				}
			}
		}

		//ランク
		GameManager::SetRank(index);
		GameObject* rank = UIFactory::CreateImage("Rank", resultCanvas, filePath[index]);
		//rank->SetActive(false);
		Image* rankImage = rank->GetComponent<Image>();
		rankImage->color.a = 0.0f;
		EasingComponent* easeRank = rank->AddComponent<EasingComponent>(&rankImage->color.a);
		EasingHandler handler;
		handler.SetWait(2.5f);
		handler.SetEasing(EaseType::Linear, 0.0f, 1.0f, 1.0f);
		easeRank->StartHandler(handler);

		//適当にボタン
		GameObject* backTitleButtonObj = UIFactory::CreateButton("BackToTitleButton", resultCanvas, L"./Data/Textures/UI/Result/back_to_title.png");
		GameObject* retryButtonObj = UIFactory::CreateButton("RetryButton", resultCanvas, L"./Data/Textures/UI/Result/retry.png");

		AudioSource* backSe = backTitleButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");
		AudioSource* retrySe = retryButtonObj->AddComponent<AudioSource>(L"./Data/Sound/SE/ui_select.wav");

		backTitleButtonObj->rect->SetAnchorMin({ 1.0f, 1.0f });
		backTitleButtonObj->rect->SetAnchorMax({ 1.0f, 1.0f });
		backTitleButtonObj->rect->SetPivot({ 1.0f, 1.0f });
		backTitleButtonObj->rect->SetAnchoredPosition({ 500, -50 });
		backTitleButtonObj->rect->size = { 500, 110 };

		retryButtonObj->rect->SetAnchorMin({ 1.0f, 1.0f });
		retryButtonObj->rect->SetAnchorMax({ 1.0f, 1.0f });
		retryButtonObj->rect->SetPivot({ 1.0f, 1.0f });
		retryButtonObj->rect->SetAnchoredPosition({ 500, -200 });
		retryButtonObj->rect->size = { 500, 110 };

		EasingComponent* easeBackButton = backTitleButtonObj->AddComponent<EasingComponent>(&backTitleButtonObj->rect->anchoredPosition.x);
		EasingHandler backHandler;
		backHandler.SetWait(3.7f);
		backHandler.SetEasing(EaseType::OutQuart, 500, -50);
		easeBackButton->StartHandler(backHandler);

		EasingComponent* easeRetryButton = retryButtonObj->AddComponent<EasingComponent>(&retryButtonObj->rect->anchoredPosition.x);
		EasingHandler retryHandler;
		retryHandler.SetWait(3.5f);
		retryHandler.SetEasing(EaseType::OutQuart, 500, -50);
		easeRetryButton->StartHandler(retryHandler);

		//タイトルへボタン
		{
			Button* titleButton = backTitleButtonObj->GetComponent<Button>();

			//ボタン押されたとき
			titleButton->AddOnClickEvent([backSe, fadeCanvas, easeFade]() {
				backSe->Play();

				//フェードしてシーン遷移
				EasingHandler handler;
				handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 1.0f);
				handler.SetCompletedFunction([]() {
					//タイトルへ
					const char* types[] = { "0", "1" };
					Scene::_transition("LoadingScene", { std::make_pair("preload", "BootScene"), std::make_pair("type", types[rand() % 2]) });
					}, true);
				fadeCanvas->SetActive(true);
				easeFade->StartHandler(handler);
				});
		}
		//リトライボタン
		{
			Button* retryButton = retryButtonObj->GetComponent<Button>();
			
			//ボタン押されたとき
			retryButton->AddOnClickEvent([retrySe, fadeCanvas, easeFade]() {
				retrySe->Play();

				//フェードしてシーン遷移
				EasingHandler handler;
				handler.SetEasing(EaseType::OutExp, 0.0f, 1.0f, 1.0f);
				handler.SetCompletedFunction([]() {
					//リトライ
					const char* types[] = { "0", "1" };
					Scene::_transition("LoadingScene", { std::make_pair("preload", "MainScene"), std::make_pair("type", types[rand() % 2]) });
					});
				fadeCanvas->SetActive(true);
				easeFade->StartHandler(handler);
				});
		}

		//タイマー表示
		ObjectManager::Find("TimerCanvas")->SetActive(true);
		ObjectManager::Find("TimerText")->GetComponent<Timer>()->SetEnable(false);

		//GameBGMストップ
		ObjectManager::Find("GameBGM")->GetComponent<AudioSource>()->Stop();

		//リザルトBGM
		GameObject* resultBgm = UIFactory::Create("ResultBGM");
		AudioSource* resultSource = resultBgm->AddComponent<AudioSource>(L"./Data/Sound/BGM/result.wav");
		resultSource->Play(XAUDIO2_LOOP_INFINITE);
		resultSource->SetVolume(0.5f);

		//フェード非表示
		ObjectManager::Find("FadeCanvas")->SetActive(false);
		fadeCanvas->SetActive(false);
	}
};