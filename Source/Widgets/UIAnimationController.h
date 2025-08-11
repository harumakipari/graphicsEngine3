#pragma once
#include "UIComponent.h"
#include "Utils/EasingHandler.h"
#include "Selectable.h"

class UIAnimationController : public UIComponent
{
public:
	enum class State { Idle, Hovered, Pressed, Disabled, Custom1, Custom2, StateCount };

	struct Transition {
		XMFLOAT2 position;
		//XMFLOAT2 size;
		float duration = 0.2f;
		EaseType type;
	private:
		friend class UIAnimationController;
		bool isRegistered = false;
	};

	void SetTransition(State state, const Transition& transition) {
		transitions[static_cast<size_t>(state)] = transition;
		transitions[static_cast<size_t>(state)].isRegistered = true;
	}


	void Update(float elapsedTime) override {

		if (Selectable* selectable = gameObject->GetComponent<Selectable>()) {
			//ステートの更新
			if (selectable->isHovered) {
				SetState(State::Hovered);
			}
			else if (selectable->isPressed) {
				SetState(State::Pressed);
			}
			else if (!selectable->IsInteractable()) {
				SetState(State::Disabled);
			}
			else {
				SetState(State::Idle);
			}


			if (transitionProgress >= 1.0f || handler.GetSequenceCount() == 0) return;

			//ハンドラ更新
			handler.Update(transitionProgress, elapsedTime);

			//座標の更新
			XMVECTOR FromPosition = XMLoadFloat2(&fromPosition);
			XMVECTOR ToPosition = XMLoadFloat2(&toPosition);
			XMFLOAT2 pos;
			XMStoreFloat2(&pos, XMVectorLerp(FromPosition, ToPosition, transitionProgress));
			rect->SetAnchoredPosition(pos);

			//サイズの更新
			/*XMVECTOR FromSize = XMLoadFloat2(&fromSize);
			XMVECTOR ToSize = XMLoadFloat2(&toSize);
			XMStoreFloat2(&rect->size, XMVectorLerp(FromSize, ToSize, transitionProgress));*/
		}
	}

	void SetState(State newState) {
		if (newState == targetState) return;
		if (!transitions[static_cast<size_t>(newState)].isRegistered) return;

		const auto& newTrans = transitions[static_cast<size_t>(newState)];
		const auto& oldTrans = transitions[static_cast<size_t>(currentState)];

		fromPosition = rect->GetWorldPosition();
		toPosition = newTrans.position;

		//fromSize = rect->size;
		//toSize = newTrans.size;

		transitionProgress = 0.0f;

		handler.Clear();
		handler.SetEasing(newTrans.type, 0.0f, 1.0f, newTrans.duration);

		currentState = targetState;
		targetState = newState;
	}

private:
	EasingHandler handler;
	State currentState;
	State targetState;

	Transition transitions[static_cast<size_t>(State::StateCount)];
	
	float transitionProgress;

	XMFLOAT2 fromPosition;
	XMFLOAT2 toPosition;

	XMFLOAT2 fromSize;
	XMFLOAT2 toSize;
};