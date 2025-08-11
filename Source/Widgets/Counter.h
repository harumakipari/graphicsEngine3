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
	//�Ď��Ώۂ��Z�b�g
	void SetListener(TutorialStep step) { this->step = step; }
	//�e�L�X�g�Z�b�g
	void SetText(Text* text, int max) { this->text = text, this->max = max; }

	//�c��J�E���g�擾
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