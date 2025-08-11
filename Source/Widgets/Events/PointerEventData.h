#pragma once
#include <DirectXMath.h>
#include "BaseEventData.h"
#include "RaycastResult.h"

//レイキャストの入力元の情報を格納するためのクラス
class PointerEventData : public BaseEventData
{
public:
#ifdef USE_MULTIPOINTER
	int pointerId = -1;					//マウス：－１
#endif // USE_MULTIPOINTER

	DirectX::XMFLOAT2 position{};			// 現在のスクリーン座標
	DirectX::XMFLOAT2 lastPosition{};		// 前フレームのスクリーン座標
	DirectX::XMFLOAT2 delta{};				// 前フレームからの移動量
	DirectX::XMFLOAT2 pressPosition{};		// 押した位置
	float scrollDelta = 0.f;				// スクロール量
	float clickTime = 0.f;					// 最後のクリック時間
	int clickCount = 0;						// クリック回数
	bool eligibleForClick = false;			// クリック候補状態
	bool dragging = false;					// ドラッグ中かどうか

	GameObject* pointerEnter = nullptr;		// 現在ホバーしているオブジェクト
	GameObject* pointerPress = nullptr;		// 押しているオブジェクト
	GameObject* lastPress = nullptr;		// 最後に押していたオブジェクト
	GameObject* pointerDrag = nullptr;		// ドラッグ対象のオブジェクト
	RaycastResult pointerCurrentRaycast;	// 現在のレイキャスト結果
	RaycastResult pointerPressRaycast;		// 押したときのレイキャスト結果

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