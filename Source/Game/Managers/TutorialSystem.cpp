#include "TutorialSystem.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


void TutorialSystem::Initialize()
{
	//パラメータリセット
	for (Step& step : steps) {
		step.Reset();
	}

	//ステップごとの追加の設定
	{
		SetOption(TutorialStep::ManyCollect, 20, 0.0f);
		SetOption(TutorialStep::SecondAttack, 0, 2.0f);
		SetOption(TutorialStep::ManyCollect2, 5, 0.0f);	
		//SetOption(TutorialStep::ThirdAttack, 2);	
		SetOption(TutorialStep::BreakBuilds, 0, 2.0f);
		SetOption(TutorialStep::BossBuild, 4, 3.0f);
	}

	//最初のステップ設定
	SetCurrentStep(TutorialStep::Start);
}

void TutorialSystem::Update(float deltaTime)
{
	if (currentStep != TutorialStep::Finish)
	{
		Step& current = steps[static_cast<size_t>(currentStep)];
		if (current.isCompleted)
		{
			current.delay -= deltaTime;
			if (current.delay < 0.0f)
			{
				//次のステップへ
				SetCurrentStep(static_cast<TutorialStep>(static_cast<size_t>(currentStep) + 1));
			}
		}
	}
}

void TutorialSystem::DrawGUI()
{
#ifdef USE_IMGUI
	ImGui::Text("CurrentStp:%d", static_cast<int>(currentStep));
#endif // USE_IMGUI
}