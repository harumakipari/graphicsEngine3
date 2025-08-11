#pragma once
#include "UIFactory.h"
#include "GameObject.h"
#include "Mask.h"
#include "Text.h"
#include "Timer.h"
#include "EasingComponent.h"
#include "BossIndicator.h"

class GameUIFactory
{
public:
	static void Create(Scene* scene, bool createTimer = true)
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
		fadeImage->color = { 0,0,0,1 };

		//FADE
		EasingComponent* easeFade = fadeCanvas->AddComponent<EasingComponent>(&fadeImage->color.a);
		EasingHandler handler;
		handler.SetEasing(EaseType::OutSine, 1.0f, 0.0f, 0.5f);
		handler.SetCompletedFunction([fadeCanvas]() {
			fadeCanvas->SetActive(false);
			}, true);
		easeFade->StartHandler(handler);


		GameObject* playerCanvas = UIFactory::CreateCanvas("PlayerCanvas");
		playerCanvas->SetActive(!createTimer);

		GameObject* leftTank = UIFactory::CreateImage("LeftGaugeTank", playerCanvas, L"./Data/Textures/UI/player_energy_frame.png", true);
		GameObject* leftGuage = UIFactory::CreateImage("LeftGauge", playerCanvas, L"./Data/Textures/UI/player_energy.png", true);
		GameObject* rightTank = UIFactory::CreateImage("RightGaugeTank", playerCanvas, L"./Data/Textures/UI/player_energy_frame.png", true);
 		GameObject* rightGuage = UIFactory::CreateImage("RightGauge", playerCanvas, L"./Data/Textures/UI/player_energy.png", true);

		GameObject* hpFrame = UIFactory::CreateImage("HPFrame", playerCanvas, L"./Data/Textures/UI/player_hp_frame.png", true);
		GameObject* hpGuage = UIFactory::CreateImage("HPGuage", playerCanvas, L"./Data/Textures/UI/player_hp.png", true);

		GameObject* playerIcon = UIFactory::CreateImage("PlayerIcon", playerCanvas, L"./Data/Textures/UI/icon_chara.png");

		//アイコンの設定
		{
			playerIcon->rect->size = { 100, 97 };
			playerIcon->rect->SetAnchorMin({ 0,1 });
			playerIcon->rect->SetAnchorMax({ 0,1 });
			playerIcon->rect->SetPivot({ 0,1 });
			playerIcon->rect->SetAnchoredPosition({ 720,-30 });
		}

		leftTank->rect->SetPivot({ 0,0 });
		rightTank->rect->SetPivot({ 0,0 });
		hpFrame->rect->SetPivot({ 0,0 });

		//親子関係構築
		leftGuage->SetParent(leftTank);
		rightGuage->SetParent(rightTank);
		hpGuage->SetParent(hpFrame);

		RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
			leftTank->rect,
			{ 0,1 },
			{ 0,1 },
			{ 0,0 }
		);
		RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
			rightTank->rect,
			{ 0,1 },
			{ 0,1 },
			{ 0,0 }
		);
		RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
			hpFrame->rect,
			{ 0,1 },
			{ 0,1 },
			{ 0,0 }
		);
		leftTank->rect->SetAnchoredPosition({ 620,-200 });
		rightTank->rect->SetAnchoredPosition({ 1110,-200 });
		hpFrame->rect->SetAnchoredPosition({ 730,-120 });
		leftTank->rect->size = { 100,180 };
		rightTank->rect->size = { 100,180 };
		hpFrame->rect->size = { 400,100 };
		
		leftGuage->rect->size = { 90, 150 };
		rightGuage->rect->size = { 90, 150 };
		hpGuage->rect->size = { 400,100 };

		//範囲値設定
		Mask* hpMask = hpGuage->GetComponent<Mask>();
		hpMask->minValue.x = 0.22f;
		hpMask->maxValue.x = 0.95f;

		Mask* energyLeftMask = leftGuage->GetComponent<Mask>();
		Mask* energyRightMask = rightGuage->GetComponent<Mask>();
		energyLeftMask->minValue.y = energyRightMask->minValue.y = 0.145f;
		energyLeftMask->maxValue.y = energyRightMask->maxValue.y = 0.828f;


		//leftGuage->AddComponent<Mask>();
		//rightGuage->AddComponent<Mask>();

		GameObject* bossCanvas = UIFactory::CreateCanvas("BossCanvas");
		GameObject* bossIcon = UIFactory::CreateImage("BossIcon", bossCanvas, L"./Data/Textures/UI/icon_boss.png");
		GameObject* bossFrame = UIFactory::CreateImage("BossHPFrame", bossCanvas, L"./Data/Textures/UI/boss_hp_frame.png", true);
		GameObject* bossHp = UIFactory::CreateImage("BossHP", bossCanvas, L"./Data/Textures/UI/boss_hp.png", true);
		GameObject* bossEnergy = UIFactory::CreateImage("BossEnergy", bossCanvas, L"./Data/Textures/UI/boss_energy.png", true);

		bossCanvas->SetActive(!createTimer);

		//Canvas
		bossCanvas->rect->SetAnchoredPosition({ 0,20 });
		bossFrame->rect->size = { 703, 79 };
		bossEnergy->rect->size = { 703, 79 };
		bossHp->rect->size = { 703, 79 };

		//色
		bossHp->GetComponent<Image>()->color = { 221 / 255.f, 35 / 255.f, 50 / 255.f, 1 };

		//アイコンの設定
		{
			bossIcon->rect->size = { 100, 100 };
			bossIcon->rect->SetAnchorMin({ 0.5f,0 });
			bossIcon->rect->SetAnchorMax({ 0.5f,0 });
			bossIcon->rect->SetPivot({ 0,0 });
			bossIcon->rect->SetAnchoredPosition({ -450,30 });
		}

		bossFrame->rect->SetPivot({ 0,0 });
		bossHp->SetParent(bossFrame);
		bossEnergy->SetParent(bossFrame);

		RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
			bossFrame->rect,
			{ 0.5f,0 },
			{ 0.5f, 0 },
			{ 0.5f, 0.f }
		);
		bossFrame->rect->SetAnchoredPosition({ 0,50 });
		bossHp->rect->SetPivot({ 1.f, 0.5f });
		bossEnergy->rect->SetPivot({ 1.f, 0.5f });

		//bossHp->AddComponent<Mask>();
		//bossEnergy->AddComponent<Mask>();
		Mask* bossHpMask = bossHp->GetComponent<Mask>();
		Mask* bossEnergyMask = bossEnergy->GetComponent<Mask>();
		bossHpMask->minValue.x = bossEnergyMask->minValue.x = 0.012f;

		//ボスインジケータ
		GameObject* bossIndicatorCanvas = UIFactory::CreateCanvas("BossIndicatorCanvas");
		{
			GameObject* bossIndicatorObj = UIFactory::CreateImage("BossIndicator", bossIndicatorCanvas, L"./Data/Textures/UI/BossIndicator/bos_icon.png");
			GameObject* bossIndicatorFrameObj = UIFactory::CreateImage("BossIndicatorFrame", bossIndicatorCanvas, L"./Data/Textures/UI/BossIndicator/bos_icon_fream.png");

			//インジケータのサイズ
			XMFLOAT2 size = { 150,150 };

			//画像設定
			{
				//アイコン
				bossIndicatorObj->rect->size = size;
				bossIndicatorObj->rect->SetAnchorMin({ 0,0 });
				bossIndicatorObj->rect->SetAnchorMax({ 0,0 });

				//親子関係
				bossIndicatorFrameObj->SetParent(bossIndicatorObj);

				//フレーム
				bossIndicatorFrameObj->rect->size = size;
				bossIndicatorFrameObj->rect->SetAnchoredPosition({ size.x * -0.5f, size.y * -0.5f });
			}

			bossIndicatorCanvas->AddComponent<BossIndicator>(bossIndicatorFrameObj);
			bossIndicatorCanvas->SetActive(false);
			bossIndicatorCanvas->priority = 1;
		}
		//タイマー
		if (createTimer)
		{
			GameObject* timerCanvas = UIFactory::CreateCanvas("TimerCanvas");
			GameObject* timerFrameObj = UIFactory::CreateImage("TimerFrame", timerCanvas, L"./Data/Textures/UI/timer_frame.png");
			//timerFrameObj->rect->size = {}
			timerFrameObj->rect->SetAnchorMin({ 1,0 });
			timerFrameObj->rect->SetAnchorMax({ 1,0 });
			timerFrameObj->rect->SetPivot({ 1,0 });
			timerFrameObj->rect->size = { 300, 175 };
			//Text* text = timerFrameObj->AddComponent<Text>(Graphics::GetDevice(), "./Data/Font/number.fnt");
			GameObject* textObj = UIFactory::CreateText("TimerText", timerCanvas, "./Data/Font/number.fnt");
			textObj->AddComponent<Timer>();
			textObj->GetComponent<Text>()->fontSize = 72.0f;
			
			textObj->rect->SetAnchorMin({ 1,0 });
			textObj->rect->SetAnchorMax({ 1,0 });
			textObj->rect->SetPivot({ 1,0 });
			textObj->rect->SetAnchoredPosition({ 62.5f,50 });
			textObj->rect->size = { 300, 175 };

			timerCanvas->SetActive(false);

			//WARNING
			GameObject* warningCanvas = UIFactory::CreateCanvas("WarningCanvas");
			GameObject* warning = UIFactory::CreateImage("Warning", warningCanvas, L"./Data/Textures/UI/WARNING.png");
			EasingHandler handler;
			handler.SetEasing(EaseType::Linear, 0.0f, 1.0f, 0.5f);
			handler.SetEasing(EaseType::Linear, 1.0f, 0.0f, 0.5f);
			handler.SetEasing(EaseType::Linear, 0.0f, 1.0f, 0.5f);
			handler.SetEasing(EaseType::Linear, 1.0f, 0.0f, 0.5f);
			handler.SetCompletedFunction([warningCanvas/*, timerCanvas, bossIndicatorCanvas*/]() {
				warningCanvas->SetActive(false);
				/*timerCanvas->SetActive(true);
				bossIndicatorCanvas->SetActive(true);*/
				}, true);
			//Easingコンポーネント
			warning->AddComponent<EasingComponent>(&warning->GetComponent<Image>()->color.a)->StartHandler(handler);
		}

	}
};