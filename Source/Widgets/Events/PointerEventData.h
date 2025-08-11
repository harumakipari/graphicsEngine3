#pragma once
#include <DirectXMath.h>
#include "BaseEventData.h"
#include "RaycastResult.h"

//���C�L���X�g�̓��͌��̏����i�[���邽�߂̃N���X
class PointerEventData : public BaseEventData
{
public:
#ifdef USE_MULTIPOINTER
	int pointerId = -1;					//�}�E�X�F�|�P
#endif // USE_MULTIPOINTER

	DirectX::XMFLOAT2 position{};			// ���݂̃X�N���[�����W
	DirectX::XMFLOAT2 lastPosition{};		// �O�t���[���̃X�N���[�����W
	DirectX::XMFLOAT2 delta{};				// �O�t���[������̈ړ���
	DirectX::XMFLOAT2 pressPosition{};		// �������ʒu
	float scrollDelta = 0.f;				// �X�N���[����
	float clickTime = 0.f;					// �Ō�̃N���b�N����
	int clickCount = 0;						// �N���b�N��
	bool eligibleForClick = false;			// �N���b�N�����
	bool dragging = false;					// �h���b�O�����ǂ���

	GameObject* pointerEnter = nullptr;		// ���݃z�o�[���Ă���I�u�W�F�N�g
	GameObject* pointerPress = nullptr;		// �����Ă���I�u�W�F�N�g
	GameObject* lastPress = nullptr;		// �Ō�ɉ����Ă����I�u�W�F�N�g
	GameObject* pointerDrag = nullptr;		// �h���b�O�Ώۂ̃I�u�W�F�N�g
	RaycastResult pointerCurrentRaycast;	// ���݂̃��C�L���X�g����
	RaycastResult pointerPressRaycast;		// �������Ƃ��̃��C�L���X�g����

public:
	PointerEventData(EventSystem* eventSystem) : BaseEventData(eventSystem) {}

	void Reset() override {
		position = {};
		lastPosition = {};
		delta = {};
		pressPosition = {};
		scrollDelta = {};
		clickTime = 0.0f;
		clickCount = 0;
		eligibleForClick = false;
		dragging = false;
		pointerEnter = nullptr;
		pointerPress = nullptr;
		lastPress = nullptr;
		pointerDrag = nullptr;
		pointerCurrentRaycast = {};
		pointerPressRaycast = {};
	}
};