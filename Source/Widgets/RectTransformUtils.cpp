#include "RectTransformUtils.h"
#include "RectTransform.h"

void RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
	RectTransform* rect,
	const XMFLOAT2& newAnchorMin,
	const XMFLOAT2& newAnchorMax,
	const XMFLOAT2& newPivot) 
{
	if (rect == nullptr || rect->GetParent() == nullptr)
		return;

	//親のサイズ
	RectTransform* parent = rect->GetParent();
	XMFLOAT2 parentSize = parent->GetWorldSize();
	XMVECTOR ParentSize = XMLoadFloat2(&parentSize);
	
	//現在の値を退避
	XMVECTOR OldAnchorMin = XMLoadFloat2(&rect->anchorMin);
	XMVECTOR OldAnchorMax = XMLoadFloat2(&rect->anchorMax);
	XMVECTOR OldPivot = XMLoadFloat2(&rect->pivot);

	XMVECTOR NewAnchorMin = XMLoadFloat2(&newAnchorMin);
	XMVECTOR NewAnchorMax = XMLoadFloat2(&newAnchorMax);
	XMVECTOR NewPivot = XMLoadFloat2(&newPivot);

	// アンカーによる位置変化補正
	XMVECTOR AnchorOffsetDelta =
		((NewAnchorMin - OldAnchorMin) * ParentSize +
			(NewAnchorMax - OldAnchorMax) * ParentSize) * 0.5f;

	// ピボットによる補正
	XMFLOAT2 size = rect->GetWorldSize();
	XMVECTOR Size = XMLoadFloat2(&size);
	XMVECTOR PivotOffsetDelta = (NewPivot - OldPivot) * Size;

	// anchoredPosition を取得して補正
	XMFLOAT2 anchoredPosition = rect->GetAnchoredPosition();
	XMVECTOR anchoredPosV = XMLoadFloat2(&anchoredPosition);
	XMVECTOR corrected = anchoredPosV - AnchorOffsetDelta + PivotOffsetDelta;

	XMFLOAT2 correctedPosition;
	XMStoreFloat2(&correctedPosition, corrected);
	rect->SetAnchoredPosition(correctedPosition);

	// 実際に Anchor と Pivot を変更
	rect->SetAnchorMin(newAnchorMin);
	rect->SetAnchorMax(newAnchorMax);
	rect->SetPivot(newPivot);
}