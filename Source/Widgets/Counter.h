#pragma once
#include "UIComponent.h"
#include "Game/Managers/TutorialSystem.h"
#include "Text.h"

class Counter : public UIComponent
{
	TutorialStep step = TutorialStep::Start;
	int count = 0;
	int max = 0;
	Text* text = nullptr;
public:
	//監視対象をセット
	void SetListener(TutorialStep step) { this->step = step; }
	//テキストセット
	void SetText(Text* text, int max) { this->text = text, this->max = max; }

	//残りカウント取得
	int GetCount() const { return count; }

	void Update(float deltaTime) override
	{
		count = TutorialSystem::GetCounter(step);

		if (text)
		{
			text->text = std::to_wstring(max - count) + L"/" + std::to_wstring(max);
		}
	}
};