#pragma once
#include <memory>
#include <vector>
#include <set>
#include <string>
#include <d3d11.h>
class GameObject;

class ObjectManager
{
	//ObjectManager() = default;
	//~ObjectManager() { objects.clear(); }
public:
	void Update(float elapsedTime);

	void Draw(ID3D11DeviceContext* immediateContext);

	void DrawHierarchy();
	void DrawProperty();

	GameObject* FindGameObject(const std::string& name);
	static GameObject* Find(const std::string& name);
	static GameObject* Find(const int& id);
	static std::shared_ptr<GameObject> Find_Ptr(const std::string& name);
	static std::shared_ptr<GameObject> Find_Ptr(const int& id);
	
	void Destroy(const std::string& name);

	bool SelectNodeExist() { return selectNode; }
	
	void Reset() {
		selectNode = nullptr;
		inspectorNode = nullptr;
	}
private:
	void DestroyChildren(GameObject* object);

	friend class Scene;
	friend class UIFactory;
	void Register(std::shared_ptr<GameObject> object) {
		objects.emplace_back(object);
	}
	GameObject* selectNode;
	GameObject* inspectorNode;
	static inline bool lockInspector = false;
public:
	std::vector<std::shared_ptr<GameObject>> objects;
	std::vector<std::shared_ptr<GameObject>> erases;
};