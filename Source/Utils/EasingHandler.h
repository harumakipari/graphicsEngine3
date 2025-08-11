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
	/// �C�[�W���O�J�n
	/// </summary>
	/// <param name="type">�C�[�W���O�^�C�v</param>
	/// <param name="start">�J�n�l</param>
	/// <param name="end">�I���l</param>
	/// <param name="duration">�⊮����(s)</param>
	void SetEasing(EaseType type, float start, float end, float duration = 1.0f, float back = 0.0f);

	/// <summary>
	/// �ҋ@�����ǉ�
	/// </summary>
	/// <param name="waitTime">�ҋ@����</param>
	void SetWait(float waitTime);

	//���Z�b�g
	void Clear();

	/// <summary>
	/// �X�V����
	/// </summary>
	/// <param name="value">�C�[�W���O����p�����[�^</param>
	/// <param name="elapsedTime">�o�ߎ���</param>
	void Update(float& value, float elapsedTime);

	//�������S�Ċ����������ǂ���
	bool IsCompleted() const { return isCompleted; }

	/// <summary>
	/// �������S�Ċ��������Ƃ��Ɏ��s����֐���ݒ�
	/// </summary>
	/// <param name="function">�ݒ肷��֐��i�����Ȃ��̂ݐݒ�\�j</param>
	void SetCompletedFunction(std::function<void()> function, bool oneShot = false) { completeFunction = function, this->oneShot = oneShot; }

	//���ݏ������Ă��鐔�擾
	size_t GetSequenceCount() const { return sequence.size(); }

private:
	//�������S�Ċ��������Ƃ��Ɏ��s����֐����o�^����Ă�����ݒ肵���֐������s����
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
