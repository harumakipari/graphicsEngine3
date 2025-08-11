#pragma once
#include <string>
#include <d3d11.h>

class RectTransform;
class GameObject;
class UIComponent
{
public:
	UIComponent() = default;
	virtual ~UIComponent() {}

	int priority = 0;

	GameObject* gameObject = nullptr; //Componentが所属するGameObject
	RectTransform* rect = nullptr;
	void SetEnable(bool set);
	bool IsEnable() const { return enable; }
	void Destroy();

private:
	friend class GameObject;
	//一番最初の初期化処理
	virtual void Awake() {};
	//初期化処理
	virtual void Initialize() {};
	//更新処理
	virtual void Update(float elapsedTime) {};
	//2D描画開始前処理
	virtual void Begin(ID3D11DeviceContext* immediateContext) {};
	//2D描画処理
	virtual void Draw(ID3D11DeviceContext* immediateContext) {};
	//2D描画終了後処理
	virtual void End(ID3D11DeviceContext* immediateContext) {};
	//デバッグGUI描画
	virtual void DrawProperty() {};
	//終了化処理
	virtual void Finalize() {};

	virtual void OnEnable() {};
	virtual void OnDisable() {};

	void SetOwner(GameObject* gameObject) { this->gameObject = gameObject; }
	void SetName(const std::string& name) { this->name = name; }

	bool enable = true;
public:
	bool hideInspector = false;//Inspectorからこのコンポーネントを隠すかどうか
	bool hideInspecterProperty = false;//Inspectorで、中の要素を隠すかどうか
public:
	//bool isRaycastTarget = false;
	std::string name;
};