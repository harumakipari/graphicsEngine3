#pragma once
#include "UIComponent.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI
#include "Text.h"

class Timer : public UIComponent
{
	float timer = 0.0f;
public:
	void Initialize()
	{
		timer = 0.0f;
	}

	void Update(float deltaTime) override
	{
		timer += deltaTime;

		if (Text* text = gameObject->GetComponent<Text>())
		{
			std::wstring min, sec;
			GetTimeWString(min, sec);
			text->text = min + L" " + sec;
		}
	}

	//タイム文字列取得
	void GetTimeWString(std::wstring& min, std::wstring& sec) const
	{
		int _time = GetTime();
		int _min = _time / 60;
		int _sec = _time % 60;
		min = std::to_wstring(_min);
		sec = std::to_wstring(_sec);

		if (_min < 10) min.insert(min.begin(), L'0');
		if (_sec < 10) sec.insert(sec.begin(), L'0');
	}

	//タイム取得
	int GetTime() const { return static_cast<int>(timer); }

	void DrawProperty() override
	{
#ifdef USE_IMGUI
		ImGui::Text("Time:%d", static_cast<int>(timer));
#endif // USE_IMGUI
	}
};