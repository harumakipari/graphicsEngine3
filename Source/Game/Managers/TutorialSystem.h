#pragma once
#include <functional>
#include "Components/Audio/AudioSourceComponent.h"
#include "Widgets/ObjectManager.h"
#include "Widgets/GameObject.h"

enum class TutorialStep
{
	Start,
	
	Move,			//移動ミッション
	Collect,		//回収ミッション
	FirstAttack,	//発射ミッション
	ManyCollect,	//チャージミッション
	SecondAttack,	//打ったら、エネルギー蓄積で機動力ダウン(Info)
	CreateBuild,	//照準ミッション (ビル耐久値あることを教える)
	ManyCollect2,	//破壊ミッション アイテムを5個集めたら
	ThirdAttack,	//ビルを衝撃波で壊す溜めのビーム
	BreakBuilds,	//破壊ミッション
	MoveCamera,		// カメラを動かす
	BossBuild,		//警告 ボスビル出して終了

	Finish,
	EnumCount
};

class TutorialSystem
{
	//チュートリアルステップ
	struct Step
	{
		bool isCompleted = false;
		int setCounter = 0;
		int counter = 0;
		float delay = 0.f;
		std::function<void()> initializeFunction;
		std::function<void()> completedFunction;
		std::function<void()> continueStepFunction;

		//パラメータリセット
		void Reset() {
			isCompleted = false;
			counter = 0;
			delay = 0.f;
			completedFunction = nullptr;
		}
	};
	static inline Step steps[static_cast<size_t>(TutorialStep::EnumCount)];
	static inline Step presets[static_cast<size_t>(TutorialStep::EnumCount)];
	static inline TutorialStep currentStep;
public:
	//初期化処理
	static void Initialize();

	//更新処理
	static void Update(float deltaTime);

	//GUI描画
	static void DrawGUI();

	//現在のステップ取得
	static TutorialStep GetCurrentStep() { return currentStep; }

	//ステップ手動設定
	static void SetCurrentStep(TutorialStep step) { 
		if (currentStep == step) {
			if (steps[static_cast<size_t>(step)].continueStepFunction) {
				steps[static_cast<size_t>(step)].continueStepFunction();
			}
		}
		currentStep = step;
		steps[static_cast<size_t>(step)] = presets[static_cast<size_t>(step)];
		if (steps[static_cast<size_t>(step)].initializeFunction) {
			steps[static_cast<size_t>(step)].initializeFunction();
		}

		//if (GameObject* check = ObjectManager::Find("Check"))
		//{
		//	check->SetActive(false);
		//}

		//音再生
		switch (step)
		{
		case TutorialStep::Start:
		case TutorialStep::Finish:
			Audio::PlayOneShot(L"./Data/Sound/SE/tutorial_popup.wav");
			break;
		default:
			break;
		}
	}

	//アクション通知
	static void AchievedAction(TutorialStep step) {
		if (currentStep == step) 
		{
			steps[static_cast<size_t>(step)].counter--;
			if (steps[static_cast<size_t>(step)].counter <= 0 && !steps[static_cast<size_t>(step)].isCompleted)
			{
				steps[static_cast<size_t>(step)].isCompleted = true;
				//完了したときに呼び出す
				if (steps[static_cast<size_t>(step)].completedFunction) 
				{
					steps[static_cast<size_t>(step)].completedFunction();
				}

				//if (GameObject* checkFrame = ObjectManager::Find("CheckFrame"))
				//{
				//	if (checkFrame->IsActive())
				//	{
				//		ObjectManager::Find("Check")->SetActive(true);
				//	}
				//}

				//音再生
				switch (step)
				{
				case TutorialStep::Move:
				case TutorialStep::Collect:
				case TutorialStep::FirstAttack:
				case TutorialStep::ManyCollect:
				case TutorialStep::SecondAttack:
				case TutorialStep::CreateBuild:
				case TutorialStep::ManyCollect2:
				case TutorialStep::ThirdAttack:
				case TutorialStep::BreakBuilds:
				case TutorialStep::MoveCamera:
				case TutorialStep::BossBuild:
					//完了音
					Audio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
					break;
				default:
					break;
				}
			}
		}
	}

	//指定のステップが開始したときに呼び出す関数設定
	static void SetInitializeFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].initializeFunction = func;
	}

	//指定のステップが完了したときに呼び出す関数設定
	static void SetCompletedFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].completedFunction = func;
	}
	//指定のステップが再びセットされたときに呼び出す関数設定
	static void SetContinueStepFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].continueStepFunction = func;
	}

	//カウンター取得
	static int GetCounter(TutorialStep step) {
		return steps[static_cast<size_t>(step)].counter;
	}
private:
	//ステップオプション設定
	static void SetOption(TutorialStep step, int counter, float nextStepDelay = 0.0f) {
		presets[static_cast<size_t>(step)].counter = counter;
		presets[static_cast<size_t>(step)].delay = nextStepDelay;
	}
};