#pragma once

#include "BaseEventData.h"
#include "PointerEventData.h"
#include "AxisEventData.h"

class IPointerEnterHandler
{
	virtual void OnPointerEnter(PointerEventData* eventData) {}
public:
	static void Execute(IPointerEnterHandler* handler, BaseEventData* eventData) {
		handler->OnPointerEnter(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IPointerExitHandler
{
	virtual void OnPointerExit(PointerEventData* eventData) {}
public:
	static void Execute(IPointerExitHandler* handler, BaseEventData* eventData) {
		handler->OnPointerExit(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IPointerUpHandler
{
	virtual void OnPointerUp(PointerEventData* eventData) {}
public:
	static void Execute(IPointerUpHandler* handler, BaseEventData* eventData) {
		handler->OnPointerUp(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IPointerDownHandler
{
	virtual void OnPointerDown(PointerEventData* eventData) {}
public:
	static void Execute(IPointerDownHandler* handler, BaseEventData* eventData) {
		handler->OnPointerDown(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IPointerClickHandler
{
	virtual void OnPointerClick(PointerEventData* eventData) {}
public:
	static void Execute(IPointerClickHandler* handler, BaseEventData* eventData) {
		handler->OnPointerClick(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IBeginDragHandler
{
	virtual void OnBeginDrag(PointerEventData* eventData) {}
public:
	static void Execute(IBeginDragHandler* handler, BaseEventData* eventData) {
		handler->OnBeginDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IDragHandler
{
	virtual void OnDrag(PointerEventData* eventData) {}
public:
	static void Execute(IDragHandler* handler, BaseEventData* eventData) {
		handler->OnDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IEndDragHandler
{
	virtual void OnEndDrag(PointerEventData* eventData) {}
public:
	static void Execute(IEndDragHandler* handler, BaseEventData* eventData) {
		handler->OnEndDrag(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IUpdateSelectedHandler
{
	virtual void OnUpdateSelected(BaseEventData* eventData) {}
public:
	static void Execute(IUpdateSelectedHandler* handler, BaseEventData* eventData) {
		handler->OnUpdateSelected(eventData);
	}
};

class IMoveHandler
{
	virtual void OnMove(AxisEventData* eventData) {}
public:
	static void Execute(IMoveHandler* handler, BaseEventData* eventData) {
		handler->OnMove(dynamic_cast<AxisEventData*>(eventData));
	}
};

class ISelectHandler
{
	virtual void OnSelect(BaseEventData* eventData) {}
public:
	static void Execute(ISelectHandler* handler, BaseEventData* eventData) {
		handler->OnSelect(eventData);
	}
};

class IDeselectHandler
{
	virtual void OnDeselect(BaseEventData* eventData) {}
public:
	static void Execute(IDeselectHandler* handler, BaseEventData* eventData) {
		handler->OnDeselect(eventData);
	}
};

class ISubmitHandler
{
	virtual void OnSubmit(BaseEventData* eventData) {}
public:
	static void Execute(ISubmitHandler* handler, BaseEventData* eventData) {
		handler->OnSubmit(eventData);
	}
};

class ICancelHandler
{
	virtual void OnCancel(BaseEventData* eventData) {}
public:
	static void Execute(ICancelHandler* handler, BaseEventData* eventData) {
		handler->OnCancel(eventData);
	}
};

class IScrollHandler
{
	virtual void OnScroll(PointerEventData* eventData) {}
public:
	static void Execute(IScrollHandler* handler, BaseEventData* eventData) {
		handler->OnScroll(dynamic_cast<PointerEventData*>(eventData));
	}
};

class IDropHandler
{
	virtual void OnDrop(PointerEventData* eventData) {}
public:
	static void Execute(IDropHandler* handler, BaseEventData* eventData) {
		handler->OnDrop(dynamic_cast<PointerEventData*>(eventData));
	}
};