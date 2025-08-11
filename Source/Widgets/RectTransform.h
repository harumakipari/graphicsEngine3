#pragma once

#include <string>

#include <d3d11.h>

#include <DirectXMath.h>
using namespace DirectX;

#include <imgui.h>
#include "../Graphics/Core/Graphics.h"

#include "UIComponent.h"
#include "RectTransformUtils.h"

class RectTransform : public UIComponent
{
public:
	DirectX::XMFLOAT2 anchoredPosition{};
	//DirectX::XMFLOAT2 sizeDelta{};
private:
	DirectX::XMFLOAT2 worldPos{};//pivot位置
	DirectX::XMFLOAT2 worldSize{};
	
	DirectX::XMFLOAT2 topLeft{};
	DirectX::XMFLOAT2 topRight{};
	DirectX::XMFLOAT2 bottomLeft{};
	DirectX::XMFLOAT2 bottomRight{};
	DirectX::XMFLOAT2 unrotatedTopLeft{};
	DirectX::XMFLOAT2 unrotatedTopRight{};
	DirectX::XMFLOAT2 unrotatedBottomLeft{};
	DirectX::XMFLOAT2 unrotatedBottomRight{};
	bool isOpen = false;//ImGui用

	DirectX::XMFLOAT2 offsetMin = { 0.f, 0.f };// anchorMinからのオフセット（ピクセル）
	DirectX::XMFLOAT2 offsetMax = { 0.f, 0.f };// anchorMaxからのオフセット（ピクセル）
public:
	float angle = 0.0f;
	DirectX::XMFLOAT2 size{};
	DirectX::XMFLOAT2 pivot = { 0.5f, 0.5f };
	DirectX::XMFLOAT2 anchorMin = { 0.5f, 0.5f };// 0～1（親の左上が (0,0)、右下が (1,1)）
	DirectX::XMFLOAT2 anchorMax = { 0.5f, 0.5f };// anchorMin == anchorMax なら固定位置
public:

	RectTransform() {}
	virtual ~RectTransform() override = default;

	RectTransform* GetParent() const;

	void Update(float elapsedTime) override {

		//中心座標とサイズ計算
		if (RectTransform* p = GetParent()) {

			XMVECTOR parentPos = XMLoadFloat2(&p->worldPos);
			XMVECTOR parentSize = XMLoadFloat2(&p->worldSize);

			XMVECTOR AnchorMin = XMLoadFloat2(&anchorMin);
			XMVECTOR AnchorMax = XMLoadFloat2(&anchorMax);

			XMVECTOR anchorRectMin = parentPos + parentSize * AnchorMin;
			XMVECTOR anchorRectMax = parentPos + parentSize * AnchorMax;

			XMVECTOR worldSizeVec;
			XMVECTOR worldPosVec;

			if (!XMVector2Equal(AnchorMin, AnchorMax)) {
				// ストレッチ（anchorMin ≠ anchorMax）
				XMVECTOR rectMin = anchorRectMin + XMLoadFloat2(&offsetMin);
				XMVECTOR rectMax = anchorRectMax + XMLoadFloat2(&offsetMax);
				worldSizeVec = rectMax - rectMin;
				worldPosVec = rectMin;
			}
			else {
				// 固定アンカー（anchorMin == anchorMax）
				worldSizeVec = XMLoadFloat2(&size);
				XMVECTOR anchorPos = anchorRectMin; // == anchorRectMax
				worldPosVec = anchorPos + XMLoadFloat2(&anchoredPosition);
			}

			XMStoreFloat2(&worldSize, worldSizeVec);
			XMStoreFloat2(&worldPos, worldPosVec);
		}
		else {
			worldSize = size;
			worldPos = anchoredPosition;
		}

		//４点の座標を求める
		{
			XMFLOAT2 min = { worldPos.x - worldSize.x * pivot.x, worldPos.y - worldSize.y * pivot.y };
			XMFLOAT2 max = { worldPos.x + worldSize.x * (1.f - pivot.x), worldPos.y + worldSize.y * (1.f - pivot.y) };
			unrotatedTopLeft = topLeft = { min };
			unrotatedTopRight = topRight = { max.x, min.y };
			unrotatedBottomLeft = bottomLeft = { min.x, max.y };
			unrotatedBottomRight = bottomRight = { max };

			Rotate(topLeft, worldPos, angle);
			Rotate(topRight, worldPos, angle);
			Rotate(bottomLeft, worldPos, angle);
			Rotate(bottomRight, worldPos, angle);

		}
	}

	void DrawProperty() override {
#ifdef USE_IMGUI

		if (isOpen) {
			isOpen = !(ImGui::Button("Cancel") || DrawAnchorPreset());
		}
		else {
			isOpen = ImGui::Button("AnchorPreset");
		}

		if (anchorMin.x != anchorMax.x || anchorMin.y != anchorMax.y) {
			float left = GetLeft();
			if (ImGui::DragFloat("Left", &left)) {
				SetLeft(left);
			}
			float right = GetRight();
			if (ImGui::DragFloat("Right", &right)) {
				SetRight(right);
			}
			float up = GetTop();
			if (ImGui::DragFloat("Up", &up)) {
				SetTop(up);
			}
			float down = GetBottom();
			if (ImGui::DragFloat("Down", &down)) {
				SetBottom(down);
			}
		}
		else {
			ImGui::DragFloat2("Pos", &anchoredPosition.x);
			ImGui::DragFloat2("Size", &size.x);
		}
		ImGui::DragFloat2("WorldPos", &worldPos.x);
		ImGui::DragFloat2("WorldSize", &worldSize.x);
		
		if (ImGui::TreeNodeEx("Anchor", ImGuiTreeNodeFlags_DefaultOpen)) {
			XMFLOAT2 min = anchorMin;
			XMFLOAT2 max = anchorMax;
			if (ImGui::DragFloat2("Min", &min.x, 0.1f, 0.0f, 1.0f) || ImGui::DragFloat2("Max", &max.x, 0.1f, 0.0f, 1.0f)) {
				RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
					this, min, max, pivot
				);
			}
			ImGui::TreePop();
		}
		XMFLOAT2 newPivot = pivot;
		if (ImGui::DragFloat2("Pivot", &newPivot.x, 0.1f)) {
			RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
				this, anchorMin, anchorMax, newPivot
			);
		}
		ImGui::DragFloat("Angle", &angle);
		//ImGui::DragFloat3("Scale", &scale.x, 0.01f);
#endif // USE_IMGUI
	}

	bool DrawAnchorPreset() {
#ifdef USE_IMGUI
		bool isChanged = false;
		struct Preset {
			const char* label;
			XMFLOAT2 min;
			XMFLOAT2 max;
		};

		// 9-point anchors
		Preset presets[12] = {
			{ "TL", {0,0}, {0,0} },
			{ "TC", {0.5f,0}, {0.5f,0} },
			{ "TR", {1,0}, {1,0} },
			{ "CL", {0,0.5f}, {0,0.5f} },
			{ "CC", {0.5f,0.5f}, {0.5f,0.5f} },
			{ "CR", {1,0.5f}, {1,0.5f} },
			{ "BL", {0,1}, {0,1} },
			{ "BC", {0.5f,1}, {0.5f,1} },
			{ "BR", {1,1}, {1,1} },
			// Stretch presets
			{ "HS", {0, 0.5f}, {1, 0.5f} }, // Horizontal Stretch
			{ "VS", {0.5f, 0}, {0.5f, 1} }, // Vertical Stretch
			{ "ST", {0, 0}, {1, 1} },       // Stretch Full
		};

		// 3x3 grid + stretch row
		for (int i = 0; i < 12; ++i) {
			ImGui::PushID(i);
			if (ImGui::Button(presets[i].label, ImVec2(30, 30))) {

				RectTransformUtils::SetAnchorAndPivotWithoutAffectingPosition(
					this,
					presets[i].min,
					presets[i].max,
					pivot
				);
				isChanged = true;
			}
			ImGui::PopID();

			// 行ごとに改行
			if (i % 3 == 2) {
				ImGui::NewLine();
			}
			else {
				ImGui::SameLine();
			}
		}
		return isChanged;
#endif // USE_IMGUI
	}


	bool Contains(const DirectX::XMFLOAT2& point) {
		XMFLOAT2 min = TopLeft();
		XMFLOAT2 max = BottomRight();
		return (min.x <= point.x && point.x <= max.x &&
			min.y <= point.y && point.y <= max.y);
	}

	DirectX::XMFLOAT2 TopLeft() const {	return topLeft;	}
	DirectX::XMFLOAT2 TopRight() const { return topRight; }
	DirectX::XMFLOAT2 BottomLeft() const { return bottomLeft; }
	DirectX::XMFLOAT2 BottomRight() const {	return bottomRight;	}

	DirectX::XMFLOAT2 UnrotatedTopLeft() const { return unrotatedTopLeft; }
	DirectX::XMFLOAT2 UnrotatedTopRight() const { return unrotatedTopRight; }
	DirectX::XMFLOAT2 UnrotatedBottomLeft() const { return unrotatedBottomLeft; }
	DirectX::XMFLOAT2 UnrotatedBottomRight() const { return unrotatedBottomRight; }

	DirectX::XMFLOAT2 ToNDC() const {
		D3D11_VIEWPORT viewport;
		UINT num{ 1 };
		Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
		DirectX::XMFLOAT2 ndc{};
		ndc.x = 2.0f * anchoredPosition.x / viewport.Width - 1.0f;
		ndc.y = 1.0f - 2.0f * anchoredPosition.y / viewport.Height;
		return ndc;
	}
	static DirectX::XMFLOAT2 ScreenToNDC(const DirectX::XMFLOAT2& anchoredPosition) {
		D3D11_VIEWPORT viewport;
		UINT num{ 1 };
		Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
		DirectX::XMFLOAT2 ndc{};
		ndc.x = 2.0f * anchoredPosition.x / viewport.Width - 1.0f;
		ndc.y = 1.0f - 2.0f * anchoredPosition.y / viewport.Height;
		return ndc;
	}
	static DirectX::XMFLOAT2 NDCToScreen(const DirectX::XMFLOAT2& ndc) {
		D3D11_VIEWPORT viewport;
		UINT num{ 1 };
		Graphics::GetDeviceContext()->RSGetViewports(&num, &viewport);
		DirectX::XMFLOAT2 screen{};
		screen.x = viewport.Width * (ndc.x + 1.0f) * 0.5f;
		screen.y = viewport.Height * (1.0f - ndc.y) * 0.5f;
		return screen;
	}

	DirectX::XMFLOAT2 GetWorldPosition() {
		Update(0.0f);
		return worldPos;
	}
	DirectX::XMFLOAT2 GetAnchoredPosition() const {
		return anchoredPosition;
	}

	DirectX::XMFLOAT2 GetWorldSize() {
		Update(0.0f);
		return worldSize;
	}
	void SetAnchorMax(const XMFLOAT2& max) { anchorMax = max; }
	void SetAnchorMin(const XMFLOAT2& min) { anchorMin = min; }
	void SetAnchoredPosition(const XMFLOAT2& pos) { anchoredPosition = pos; }

	void SetPivot(const XMFLOAT2& pivot) { this->pivot = pivot; }
	XMFLOAT2 GetPivot() const { return pivot; }

	//void SetSizeDelta(const XMFLOAT2& delta) { sizeDelta = delta; }
	//XMFLOAT2 GetSizeDelta() const { return sizeDelta; }

	void SetLeft(float left) { offsetMin.x = left; }
	void SetRight(float right) { offsetMax.x = right; }
	void SetTop(float top) { offsetMin.y = top; }
	void SetBottom(float bottom) { offsetMax.y = bottom; }

	float GetLeft() const { return offsetMin.x; }
	float GetRight() const { return offsetMax.x; }
	float GetTop() const { return offsetMin.y; }
	float GetBottom() const { return offsetMax.y; }

private:
	void Rotate(XMFLOAT2& point, XMFLOAT2 center, float angle)
	{
		point.x -= center.x;
		point.y -= center.y;

		float cos{ cosf(DirectX::XMConvertToRadians(angle)) };
		float sin{ sinf(DirectX::XMConvertToRadians(angle)) };
		float tx{ point.x }, ty{ point.y };
		point.x = cos * tx + -sin * ty;
		point.y = sin * tx + cos * ty;

		point.x += center.x;
		point.y += center.y;
	}
};