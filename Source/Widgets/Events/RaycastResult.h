#pragma once
#include <DirectXMath.h>
class GameObject;
class GraphicRaycaster;

struct RaycastResult
{
	GameObject* gameObject = nullptr;//ヒットしたGameObject(UI要素)

	float distance = 0.0f;//使ってない
	DirectX::XMFLOAT3 worldPosition;//使ってない
	DirectX::XMFLOAT3 worldNormal;//使ってない

	DirectX::XMFLOAT2 screenPosition;

	int sortingLayer = 0;//まだ使ってない
	int sortingOrder = 0;//まだ使ってない
	int depth = 0;//まだ使ってない
	GraphicRaycaster* module = nullptr;

	bool IsValid() const {
		return gameObject != nullptr;
	}
};