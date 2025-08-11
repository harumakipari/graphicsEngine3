#include "RectTransform.h"
#include "GameObject.h"

RectTransform* RectTransform::GetParent() const {
	//親がいなかったらnullを返す
	if (!gameObject->parent)
		return nullptr;
	//親オブジェクトのRectTransformを返す
	return gameObject->parent->rect;
}