#include "RectTransform.h"
#include "GameObject.h"

RectTransform* RectTransform::GetParent() const {
	//�e�����Ȃ�������null��Ԃ�
	if (!gameObject->parent)
		return nullptr;
	//�e�I�u�W�F�N�g��RectTransform��Ԃ�
	return gameObject->parent->rect;
}