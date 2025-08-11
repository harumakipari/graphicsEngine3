#pragma once
#include <DirectXMath.h>
#include "BaseEventData.h"

enum MoveDirection {
	Up,
	Left,
	Down,
	Right,
	None
};


class AxisEventData : public BaseEventData
{
public:
	MoveDirection moveDir = MoveDirection::None;
	DirectX::XMFLOAT2 moveVector{};
public:
	AxisEventData(EventSystem* eventSystem) : BaseEventData(eventSystem) {}

	void SetDirection(DirectX::XMFLOAT2 dir) {
		moveVector = dir;
		MoveDirection old = moveDir;
		moveDir = (dir.x == 0 && dir.y == 0) ? None :
			dir.x < 0 ? Left :
			abs(dir.y) < 0.01f ? Right :
			dir.y > 0 ? Up :
			Down;
	}

	MoveDirection GetDirection() const { return moveDir; }
};