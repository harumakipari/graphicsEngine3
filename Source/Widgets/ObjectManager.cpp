#include "ObjectManager.h"
#include <functional>

#include "GameObject.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

#include "../Engine/Scene/Scene.h"

void ObjectManager::Update(float elapsedTime)
{

	if (!erases.empty()) {
		objects.erase(std::remove_if(objects.begin(), objects.end(),
			[&](const auto& obj) {
				return std::find(erases.begin(), erases.end(), obj) != erases.end();
			}),
			objects.end());
		erases.clear();
	}
	//優先度でソート
	std::sort(objects.begin(), objects.end(),
		[](const std::shared_ptr<GameObject>& a, const std::shared_ptr<GameObject>& b) {
			return a->priority < b->priority;
		});

	std::function<void(float, GameObject*)> Update = [&](float elapsedTime, GameObject* object)
		{
			object->Update(elapsedTime);
			for (auto child : object->children) {
				Update(elapsedTime, child);
			}
		};
	for (auto& object : objects) {
		if (!object || object->parent) continue;
		Update(elapsedTime, object.get());
	}
}

void ObjectManager::Draw(ID3D11DeviceContext* immediateContext)
{
	std::function<void(ID3D11DeviceContext*, GameObject*)> Draw = [&](ID3D11DeviceContext* immediateContext, GameObject* object)
		{
			object->Begin(immediateContext);
			object->Draw(immediateContext);
			object->End(immediateContext);
			for (auto child : object->children) {
				Draw(immediateContext, child);
			}
		};
	for (auto& object : objects) {
		if (!object || object->parent) continue;
		Draw(immediateContext, object.get());
	}
}

void ObjectManager::DrawHierarchy()
{
#ifdef USE_IMGUI
	//ImVec2 pos = ImGui::GetWindowPos();
	//ImGui::SetNextWindowPos(ImVec2(pos.x - 40, pos.y - 30), ImGuiCond_Once);
	//ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_Once);

	//if (ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_None))
	{
		int i = 0;
		std::function<void(GameObject*)> DrawNodeTree = [&](GameObject* object)
			{
				ImGui::PushID(i++);

				//矢印をクリックするか、ノードをタブルクリックで階層を開く。当たり判定は余白も含める
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick
					| ImGuiTreeNodeFlags_FramePadding;

				//子がいない場合は矢印をつけない
				size_t childCount = object->children.size();
				if (childCount == 0) {
					nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				}

				//選択フラグ
				if (selectNode == object) {
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//ツリーノードを描画
				float alpha = 1;//uniqueId-isVisible
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, alpha));
				bool isActive = object->IsActive();
				if (ImGui::Checkbox("##UI", &isActive))
					object->SetActive(isActive);
				ImGui::PopStyleColor();
				ImGui::SameLine();
				float textColor = object->IsActive() ? 1.0f : 0.5f;
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(textColor, textColor, textColor, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
				bool opened = ImGui::TreeNodeEx(object, nodeFlags, object->name.c_str());
				ImGui::PopStyleColor(4);
				//フォーカスされたノードを選択する
				if (ImGui::IsItemClicked()) {
					selectNode = object;
					//InspectorがロックされてなかったらInspectorの表示ノード設定
					if (!lockInspector) {
						inspectorNode = selectNode;
					}
				}
				if (opened) {
					//GameObject*データとしてドラッグ
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						ImGui::SetDragDropPayload("GameObject", &selectNode->id, sizeof(int*));
						ImGui::Text(selectNode->name.c_str());
						ImGui::EndDragDropSource();
					}
				}
				ImGui::PopID();
				//開かれている場合、子階層にも同じ処理をする
				if (opened && childCount > 0) {
					for (GameObject* child : object->children) {
						DrawNodeTree(child);
					}
					ImGui::TreePop();
				}
			};
		
		for (auto& object : objects) {
			//開かれている場合、子階層にも同じ処理をする
			if (!object || object->parent) continue;
			DrawNodeTree(object.get());
		}
	}
	//ImGui::End();
#endif // USE_IMGUI
}

void ObjectManager::DrawProperty()
{
#ifdef USE_IMGUI
	/*ImVec2 pos = ImGui::GetWindowPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 900, pos.y - 30), ImGuiCond_Once);
	ImGui::SetNextWindowSize(ImVec2(300, 600), ImGuiCond_Once);

	if (ImGui::Begin("Inspector"))*/
	{
		if (inspectorNode)
		{
			//Inspectorロック
			ImGui::Checkbox("Lock", &lockInspector);

			bool isActive = inspectorNode->IsActive();
			if (ImGui::Checkbox("##Property", &isActive)) {
				inspectorNode->SetActive(isActive);
			}
			ImGui::SameLine();

#if 1
			static size_t bufferSize = 256;
			static char buffer[256] = "";
			if (!ImGui::IsItemEdited()) {
				strncpy_s(buffer, inspectorNode->name.c_str(), bufferSize);
				buffer[bufferSize - 1] = '\0';
			}
			ImGui::InputText("GameObjectName", buffer, sizeof(buffer), ImGuiInputTextFlags_AutoSelectAll);
			if (ImGui::IsItemEdited()) {
				inspectorNode->name = buffer;
			}
#else
			ImGui::Text(inspectorNode->name.c_str());
#endif		
			ImGui::Separator();
			inspectorNode->DrawProperty();
		}
	}
	//ImGui::End();
#endif // USE_IMGUI
}
GameObject* ObjectManager::FindGameObject(const std::string& name)
{
	for (auto& object : objects) {
		if (object->name == name) {
			return object.get();
		}
	}
	return nullptr;
}
GameObject* ObjectManager::Find(const std::string& name)
{
	if (Scene* scene = Scene::GetCurrentScene())
	{
		for (auto& object : scene->objectManager.objects) {
			if (object->name == name) {
				return object.get();
			}
		}
	}
	return nullptr;
}
GameObject* ObjectManager::Find(const int& id)
{
	if (Scene* scene = Scene::GetCurrentScene())
	{
		for (auto& object : scene->objectManager.objects) {
			if (object->id == id) {
				return object.get();
			}
		}
	}
	return nullptr;
}

std::shared_ptr<GameObject> ObjectManager::Find_Ptr(const std::string& name)
{
	if (Scene* scene = Scene::GetCurrentScene())
	{
		for (auto& object : scene->objectManager.objects) {
			if (object->name == name) {
				return object;
			}
		}
	}
	return nullptr;
}
std::shared_ptr<GameObject> ObjectManager::Find_Ptr(const int& id)
{
	if (Scene* scene = Scene::GetCurrentScene())
	{
		for (auto& object : scene->objectManager.objects) {
			if (object->id == id) {
				return object;
			}
		}
	}
	return nullptr;
}

void ObjectManager::Destroy(const std::string& name) {
	std::shared_ptr<GameObject> object = Find_Ptr(name);
	if (object) {
		object->SetActive(false);
		erases.emplace_back(object);

		if (inspectorNode == object.get()) {
			Reset();
		}
		DestroyChildren(object.get());
	}
}

void ObjectManager::DestroyChildren(GameObject* object) {
	if (object) {
		for (GameObject* child : object->children) {
			Destroy(child->name);
		}
	}
}