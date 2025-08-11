#pragma once

#include <DirectXMath.h>
using namespace DirectX;
class RectTransform;

class RectTransformUtils
{
public:
	static void SetAnchorAndPivotWithoutAffectingPosition(
		RectTransform* rect,
		const XMFLOAT2& newAnchorMin,
		const XMFLOAT2& newAnchorMax,
		const XMFLOAT2& newPivot
	);
};
