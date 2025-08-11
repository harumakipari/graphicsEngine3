#pragma once
#include "easing.h"
#include <functional>
#include <vector>

enum EaseType
{
	InQuad,
	OutQuad,
	InOutQuad,
	InCubic,
	OutCubic,
	InOutCubic,
	InQuart,
	OutQuart,
	InOutQuart,
	InQuint,
	OutQuint,
	InOutQuint,
	InSine,
	OutSine,
	InOutSine,
	InExp,
	OutExp,
	InOutExp,
	InCirc,
	OutCirc,
	InOutCirc,
	InBounce,
	OutBounce,
	InOutBounce,
	InBack,
	OutBack,
	InOutBack,
	Linear
};

class EasingHandler
{
public:
	EasingHandler() {}
	~EasingHandler() {}

	/// <summary>
	/// イージング開始
	/// </summary>
	/// <param name="type">イージングタイプ</param>
	/// <param name="start">開始値</param>
	/// <param name="end">終了値</param>
	/// <param name="duration">補完時間(s)</param>
	void SetEasing(EaseType type, float start, float end, float duration = 1.0f, float back = 0.0f);

	/// <summary>
	/// 待機処理追加
	/// </summary>
	/// <param name="waitTime">待機時間</param>
	void SetWait(float waitTime);

	//リセット
	void Clear();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="value">イージングするパラメータ</param>
	/// <param name="elapsedTime">経過時間</param>
	void Update(float& value, float elapsedTime);

	//処理が全て完了したかどうか
	bool IsCompleted() const { return isCompleted; }

	/// <summary>
	/// 処理が全て完了したときに実行する関数を設定
	/// </summary>
	/// <param name="function">設定する関数（引数なしのみ設定可能）</param>
	void SetCompletedFunction(std::function<void()> function, bool oneShot = false) { completeFunction = function, this->oneShot = oneShot; }

	//現在処理している数取得
	size_t GetSequenceCount() const { return sequence.size(); }

private:
	//処理が全て完了したときに実行する関数が登録されていたら設定した関数を実行する
	void ExecuteCompletedFunction() { 
		if (completeFunction != nullptr) {
			completeFunction();
			if (oneShot) {
				completeFunction = nullptr;
			}
		}
	}

private:
	struct EaseItem
	{
		EaseData easeData{};
		std::function<float(float, float, float, float)> function;
		std::function<float(float, float, float, float, float)> backFunction;
	};
	std::vector<EaseItem> sequence;
	bool isCompleted = false;
	std::function<void()> completeFunction;
	bool oneShot = false;
};
