#pragma once
#include "UIComponent.h"
#include "Game/Scenes/MainScene.h"
#include "Image.h"
class BossIndicator : public UIComponent
{
	GameObject* bindObject = nullptr;
public:
	BossIndicator(GameObject* bindImage) : bindObject(bindImage) {}

	void Update(float deltaTime) override
	{
		if (MainScene* scene = dynamic_cast<MainScene*>(Scene::GetCurrentScene()))
		{
			if (scene->IsFrameOutEnemy())
			{
				XMFLOAT2 pos = scene->GetCursorIntersectPos();
				XMVECTOR Pos = XMLoadFloat2(&pos);
				XMFLOAT2 screenSize = rect->size;
				float thresholdX = 100.0f;
				float thresholdY = 225.0f;
				Pos = XMVectorClamp(Pos, { thresholdX,thresholdY,0,0 }, { screenSize.x - thresholdX , screenSize.y - thresholdY ,0,0 });
				XMStoreFloat2(&pos, Pos);
				bindObject->parent->rect->SetAnchoredPosition(pos);
				bindObject->parent->SetActive(true);
				bindObject->rect->angle = scene->GetAngleCursorDegree();
			}
			else
			{
				bindObject->parent->SetActive(false);
			}
		}
	}
};