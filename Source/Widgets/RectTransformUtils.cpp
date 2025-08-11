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

	//�e�̃T�C�Y
	RectTransform* parent = rect->GetParent();
	XMFLOAT2 parentSize = parent->GetWorldSize();
	XMVECTOR ParentSize = XMLoadFloat2(&parentSize);
	
	//���݂̒l��ޔ�
	XMVECTOR OldAnchorMin = XMLoadFloat2(&rect->anchorMin);
	XMVECTOR OldAnchorMax = XMLoadFloat2(&rect->anchorMax);
	XMVECTOR OldPivot = XMLoadFloat2(&rect->pivot);

	XMVECTOR NewAnchorMin = XMLoadFloat2(&newAnchorMin);
	XMVECTOR NewAnchorMax = XMLoadFloat2(&newAnchorMax);
	XMVECTOR NewPivot = XMLoadFloat2(&newPivot);

	// �A���J�[�ɂ��ʒu�ω��␳
	XMVECTOR AnchorOffsetDelta =
		((NewAnchorMin - OldAnchorMin) * ParentSize +
			(NewAnchorMax - OldAnchorMax) * ParentSize) * 0.5f;

	// �s�{�b�g�ɂ��␳
	XMFLOAT2 size = rect->GetWorldSize();
	XMVECTOR Size = XMLoadFloat2(&size);
	XMVECTOR PivotOffsetDelta = (NewPivot - OldPivot) * Size;

	// anchoredPosition ���擾���ĕ␳
	XMFLOAT2 anchoredPosition = rect->GetAnchoredPosition();
	XMVECTOR anchoredPosV = XMLoadFloat2(&anchoredPosition);
	XMVECTOR corrected = anchoredPosV - AnchorOffsetDelta + PivotOffsetDelta;

	XMFLOAT2 correctedPosition;
	XMStoreFloat2(&correctedPosition, corrected);
	rect->SetAnchoredPosition(correctedPosition);

	// ���ۂ� Anchor �� Pivot ��ύX
	rect->SetAnchorMin(newAnchorMin);
	rect->SetAnchorMax(newAnchorMax);
	rect->SetPivot(newPivot);
}