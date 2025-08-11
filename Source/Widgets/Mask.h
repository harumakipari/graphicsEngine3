#pragma once
#include <DirectXMath.h>
#include "UIComponent.h"
#include "RectTransform.h"
#include "Graphics/Core/Graphics.h"
#include "Graphics/Core/RenderState.h"

class Mask : public UIComponent
{
public:
	float valueX = 1.0f;
	float valueY = 1.0f;
	XMFLOAT2 minValue{ 0,0 };
	XMFLOAT2 maxValue{ 1,1 };

	void Begin(ID3D11DeviceContext* immediateContext) override {
#if 1
		RectTransform* maskRect = gameObject->rect;
		RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::USE_SCISSOR_RECTS);
		D3D11_VIEWPORT viewport{};
		UINT numViewports{ 1 };
		immediateContext->RSGetViewports(&numViewports, &viewport);
		//DirectX::XMFLOAT2 _pos = maskRect->GetWorldPosition();
		//DirectX::XMFLOAT2 _size = maskRect->GetWorldSize();
		D3D11_RECT scissorRect{};

		XMFLOAT2 size = { maskRect->UnrotatedBottomRight().x - maskRect->UnrotatedTopLeft().x, maskRect->UnrotatedBottomRight().y - maskRect->UnrotatedTopLeft().y };
		size.x *= (valueX * (maxValue.x - minValue.x) + minValue.x);
		size.y *= (valueY * (maxValue.y - minValue.y) + minValue.y);

		float left = maskRect->UnrotatedTopLeft().x;
		float right = left + size.x;
		float top = maskRect->UnrotatedBottomRight().y - size.y;
		float bottom = maskRect->UnrotatedBottomRight().y;

		scissorRect.left = static_cast<LONG>(min(left, right));
		scissorRect.right = static_cast<LONG>(max(left, right));
		scissorRect.top = static_cast<LONG>(min(top, bottom));
		scissorRect.bottom = static_cast<LONG>(max(top, bottom));

		immediateContext->RSSetScissorRects(1, &scissorRect);
#endif
	}

	void End(ID3D11DeviceContext* immediateContext) override {
		RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);
	}

	void DrawProperty() override {
#ifdef USE_IMGUI
		ImGui::SliderFloat("valueX", &valueX, 0.f, 1.f);
		ImGui::SliderFloat("valueY", &valueY, 0.f, 1.f);
		ImGui::DragFloat2("minVlaue", &minValue.x, 0.01f, 0.0f, 1.0f);
		ImGui::DragFloat2("maxVlaue", &maxValue.x, 0.01f, 0.0f, 1.0f);
#endif // !USE_IMGUI
	}
};