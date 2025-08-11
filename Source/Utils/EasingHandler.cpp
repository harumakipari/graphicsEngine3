#include "EasingHandler.h"

void EasingHandler::SetEasing(EaseType type, float start, float end, float duration, float back)
{
	//処理内容を設定
	EaseItem item{};

	switch (type)
	{
	case InQuad: item.function = Easing::InQuad<float>; break;
	case OutQuad: item.function = Easing::OutQuad<float>; break;
	case InOutQuad:item.function = Easing::InOutQuad<float>; break;
	case InCubic:item.function = Easing::InCubic<float>; break;
	case OutCubic:item.function = Easing::OutCubic<float>; break;
	case InOutCubic:item.function = Easing::InOutCubic<float>; break;
	case InQuart:item.function = Easing::InQuart<float>; break;
	case OutQuart:item.function = Easing::OutQuart<float>; break;
	case InOutQuart:item.function = Easing::InOutQuart<float>; break;
	case InQuint:item.function = Easing::InQuint<float>; break;
	case OutQuint:item.function = Easing::OutQuint<float>; break;
	case InOutQuint:item.function = Easing::InOutQuint<float>; break;
	case InSine:item.function = Easing::InSine<float>; break;
	case OutSine:item.function = Easing::OutSine<float>; break;
	case InOutSine:item.function = Easing::InOutSine<float>; break;
	case InExp:item.function = Easing::InExp<float>; break;
	case OutExp:item.function = Easing::OutExp<float>; break;
	case InOutExp:item.function = Easing::InOutExp<float>; break;
	case InCirc:item.function = Easing::InCirc<float>; break;
	case OutCirc:item.function = Easing::OutCirc<float>; break;
	case InOutCirc:item.function = Easing::InOutCirc<float>; break;
	case InBounce:item.function = Easing::InBounce<float>; break;
	case OutBounce:item.function = Easing::OutBounce<float>; break;
	case InOutBounce:item.function = Easing::InOutBounce<float>; break;
	case Linear:item.function = Easing::Linear<float>; break;
	case InBack:item.backFunction = Easing::InBack<float>; break;
	case OutBack:item.backFunction = Easing::OutBack<float>; break;
	case InOutBack:item.backFunction = Easing::InOutBack<float>; break;
	default:
		break;
	}
	item.easeData.timer = 0.0f;
	item.easeData.totalTime = duration;
	item.easeData.startValue = start;
	item.easeData.endValue = end;

	if (item.backFunction) {
		item.easeData.backValue = back;
	}

	//シーケンスに追加
	sequence.emplace_back(item);

	isCompleted = false;
}

void EasingHandler::SetWait(float waitTime)
{
	//処理内容を設定
	EaseItem item{};
	item.function = nullptr;
	item.easeData.timer = 0.0f;
	item.easeData.totalTime = waitTime;

	//シーケンスに追加
	sequence.emplace_back(item);
	
	isCompleted = false;
}

void EasingHandler::Update(float& value, float elapsedTime)
{
	if (!sequence.size()) return;

	auto& item = sequence.at(0);

	//先頭のイージング処理を実行する
	{
		item.easeData.timer += elapsedTime;

		//イージング関数
		if (item.function != nullptr)
			value = item.function(item.easeData.timer, item.easeData.totalTime, item.easeData.endValue, item.easeData.startValue);
		else if (item.backFunction != nullptr) {
			value = item.backFunction(item.easeData.timer, item.easeData.totalTime, item.easeData.backValue, item.easeData.endValue, item.easeData.startValue);
		}

		if (item.easeData.timer > item.easeData.totalTime)
		{
			if (item.function != nullptr || item.backFunction != nullptr)
				value = item.easeData.endValue;
			sequence.erase(sequence.begin());
		}
	}
	//全ての補完処理が完了したら完了フラグを立てる
	if (sequence.empty() && !isCompleted)
	{
		isCompleted = true;
		sequence.clear();
		ExecuteCompletedFunction();
		return;
	}
}

void EasingHandler::Clear()
{
	sequence.clear();
	isCompleted = false;
	completeFunction = nullptr;
}