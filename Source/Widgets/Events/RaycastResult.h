#pragma once
#include <DirectXMath.h>
class GameObject;
class GraphicRaycaster;

struct RaycastResult
{
	GameObject* gameObject = nullptr;//�q�b�g����GameObject(UI�v�f)

	float distance = 0.0f;//�g���ĂȂ�
	DirectX::XMFLOAT3 worldPosition;//�g���ĂȂ�
	DirectX::XMFLOAT3 worldNormal;//�g���ĂȂ�

	DirectX::XMFLOAT2 screenPosition;

	int sortingLayer = 0;//�܂��g���ĂȂ�
	int sortingOrder = 0;//�܂��g���ĂȂ�
	int depth = 0;//�܂��g���ĂȂ�
	GraphicRaycaster* module = nullptr;

	bool IsValid() const {
		return gameObject != nullptr;
	}
};