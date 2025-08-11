#pragma once
#include "UIComponent.h"
#include "GameObject.h"

class Graphic;

class Canvas : public UIComponent
{
	std::vector<Graphic*> graphics;
	std::vector<Graphic*> erases;
public:
	Canvas() = default;
	~Canvas() override = default;

	void Initialize() override {
		rect->size = { Graphics::GetScreenWidth(), Graphics::GetScreenHeight() };
	}

	void Update(float elapsedTime) override {
		if (!erases.empty()) {
			graphics.erase(std::remove_if(graphics.begin(), graphics.end(),
				[&](const auto& graphic) {
					return std::find(erases.begin(), erases.end(), graphic) != erases.end();
				}),
				graphics.end());
			erases.clear();
		}

		//画面サイズ更新
		rect->size = { Graphics::GetScreenWidth(), Graphics::GetScreenHeight() };
	}

	std::vector<Graphic*> GetGraphics() const {
		return graphics;
	}

	void RegisterGraphic(Graphic* graphic) {
		if (std::find(graphics.begin(), graphics.end(), graphic) == graphics.end())
			graphics.emplace_back(graphic);
	}
	void UnregisterGraphic(Graphic* graphic) {
		if (std::find(erases.begin(), erases.end(), graphic) == erases.end())
			erases.emplace_back(graphic);
	}


	void DrawProperty() override {
#ifdef USE_IMGUI
		ImGui::Text("GraphicsCount:%d", static_cast<int>(graphics.size()));
#endif // USE_IMGUI
	}
};