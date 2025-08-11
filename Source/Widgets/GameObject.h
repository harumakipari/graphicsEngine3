#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include "UIComponent.h"
#include "RectTransform.h"
#include "ObjectManager.h"
//#include "Events/EventSystem.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

class GameObject
{
public:
    GameObject() = default;
    virtual ~GameObject() {
        for (auto child : children) {
            child->parent = nullptr;
        }
        SetParent(nullptr);
    }
    GameObject(const GameObject&) = default;

    void Create(const std::string& name) {
        static int uniqueID = 0;
        id = uniqueID++;
        this->name = name;
        MakeUniqueName(this->name);
        isCreated = true;
        rect = AddComponent<RectTransform>();
    }

    void MakeUniqueName(std::string& name) {
        std::string addName;
        int i = 1;
        while (ObjectManager::Find(name + addName)) {
            addName = "(" + std::to_string(i++) + ")";
        }
        name += addName;
    }

    /**
    * @brief �w�肵���^�̃R���|�[�l���g��ǉ����܂��B
    * @tparam T �R���|�[�l���g�̌^�B
    * @tparam Args �R���X�g���N�^�����̌^�B
    * @param args T �̃R���X�g���N�^�ɓn���������B
    * @return �ǉ����ꂽ T �̃|�C���^�B
    */
    template<typename T, typename... Args>
    T* AddComponent(Args... args) {
        std::shared_ptr<UIComponent> component = std::shared_ptr<T>(new T(args...));
        std::shared_ptr<T> instance = std::dynamic_pointer_cast<T>(component);
        _components.emplace_back(component);
        component->SetOwner(this);
        component->rect = rect;
        std::string className = typeid(T).name();
        className = className.substr(className.find_last_of(" ") + 1, className.length());
        component->SetName(className);
        component->Awake();
        component->SetEnable(true);
        component->Initialize();
        return instance.get();
    }

    /// <summary>
    /// �R���|�[�l���g���擾
    /// </summary>
    /// <typeparam name="T">�N���X��</typeparam>
    /// <returns>�R���|�[�l���g�N���X�̃|�C���^�[</returns>
    template<typename T>
    T* GetComponent() {
        for (auto& component : _components) {
            if (T* p = dynamic_cast<T*>(component.get())) {
                return p;
            }
        }
        return nullptr;
    }
    /// <summary>
    /// �X�}�[�g�|�C���^�̃R���|�[�l���g���擾
    /// </summary>
    /// <typeparam name="T">�N���X��</typeparam>
    /// <returns>�R���|�[�l���g�N���X�̃|�C���^�[</returns>
    template<typename T>
    std::shared_ptr<T> GetComponentShared() {
        for (auto& component : _components) {
            if (std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component)) {
                return p;
            }
        }
        return nullptr;
    }
    /// <summary>
    /// �w��̃R���|�[�l���g�����ׂĎ擾
    /// </summary>
    /// <typeparam name="T">�N���X��</typeparam>
    /// <returns>�R���|�[�l���g�|�C���^��vector</returns>
    template<typename T>
    std::vector<T*> GetComponents() {
        std::vector<T*> components;
        for (auto& component : _components) {
            if (T* p = dynamic_cast<T*>(component.get())) {
                components.push_back(p);
            }
        }
        return components;
    }
    /// <summary>
    /// �e�I�u�W�F�N�g����R���|�[�l���g�擾
    /// </summary>
    /// <typeparam name="T">��������R���|�[�l���g�N���X</typeparam>
    /// <returns>�擾�����R���|�[�l���g�N���X�̃|�C���^�[</returns>
    template<typename T>
    T* GetComponentInParent() {
        if (GameObject* root = this->parent) {
            while (root->parent) {
                root = root->parent;
            }
            for (auto& component : root->_components) {
                if (T* p = dynamic_cast<T*>(component.get())) {
                    return p;
                }
            }
        }
        return nullptr;
    }

    template<typename T>
    T* GetComponentInChildren() {
        if (T* component = this->GetComponent<T>()) {
            return component;
        }
        for (GameObject* child : this->children) {
            if (T* component = child->GetComponent<T>()) {
                return component;
            }
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<UIComponent>> GetAllComponents() {
        return _components;
    }

    template<typename T>
    void RemoveComponent() {
        std::string className = typeid(T).name();
        className = className.substr(className.find_last_of(" ") + 1, className.length());
        for (auto& component : _components) {
            if (std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component)) {
                removes.push_back(p);
            }
        }
    }

    //�e�I�u�W�F�N�g�ݒ�
    void SetParent(GameObject* newParent) {
        if (parent) { //���łɐe���ݒ肳��Ă�����A���̐e�̎q���玩�g���폜
            auto& children = parent->children;
            children.erase(std::remove(children.begin(), children.end(), this), children.end());
        }
        parent = newParent;
        if (parent) {
            newParent->children.push_back(this);
            SetActive(parent->IsActive());
        }
    }
private:
    friend class ObjectManager;
    friend class Canvas;
    //���ׂẴR���|�[�l���g��Update�֐����Ăяo��
    void Update(float elapsedTime) {
        if (!removes.empty()) {
            std::unordered_set<std::shared_ptr<UIComponent>> removeSet(removes.begin(), removes.end());
            _components.erase(std::remove_if(_components.begin(), _components.end(),
                [&removeSet](const std::shared_ptr<UIComponent>& ptr) {
                    return removeSet.count(ptr) > 0;
                }),
                _components.end());
            removes.clear();
        }
        //�D��x�Ń\�[�g
        std::sort(_components.begin(), _components.end(),
            [](const std::shared_ptr<UIComponent>& a, const std::shared_ptr<UIComponent>& b) {
                if (a == nullptr && b == nullptr)
                    return false;
                if (a == nullptr) return false;
                if (b == nullptr) return true;
                return a->priority < b->priority;
            });

        if (isActive) {
            for (auto& component : _components) {
                if (component != nullptr)
                {
                    if (component->enable)
                        component->Update(elapsedTime);
                }
            }
        }
    }

    //���ׂẴR���|�[�l���g��2D�`��O����
    void Begin(ID3D11DeviceContext* immediateContext) {
        if (isActive) {
            for (auto& component : _components) {
                if (component->enable) {
                    component->Begin(immediateContext);
                }
            }
        }
    }
    //���ׂẴR���|�[�l���g��2D�`�揈��
    void Draw(ID3D11DeviceContext* immediateContext) {
        if (isActive) {
            for (auto& component : _components) {
                if (component->enable) {
                    component->Draw(immediateContext);
                }
            }
        }
    }
    //���ׂẴR���|�[�l���g��2D�`��㏈��
    void End(ID3D11DeviceContext* immediateContext) {
        if (isActive) {
            for (auto& component : _components) {
                if (component->enable) {
                    component->End(immediateContext);
                }
            }
        }
    }

    //���ׂẴR���|�[�l���g�̃v���p�e�B�`��
    void DrawProperty() {
#ifdef USE_IMGUI
        for (auto& component : _components) {
            if (component->hideInspector) continue;
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.75f, 0.75f, 0.75f, 0.75f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));
            if (ImGui::TreeNodeEx(component->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Separator();
                if (!component->hideInspecterProperty)
                    component->DrawProperty();
                ImGui::TreePop();
            }
            ImGui::PopStyleColor(3);
            ImGui::Separator();
        }
#endif // USE_IMGUI
    }
public:

    bool IsActive() const { return isActive; }

    void SetActive(bool set) {
        isActive = set;
        for (auto& component : _components) {
            component->SetEnable(set);
        }
        for (GameObject* child : children) {
            child->SetActive(set);
        }
    }

    void Destroy(UIComponent* component) {
        for (auto& p : _components) {
            if (p.get() == component) {
                removes.push_back(p);
            }
        }
    }

public:
    int id;
    std::string name;
    int priority = 0;

    RectTransform* rect = nullptr;

    GameObject* parent = nullptr;
    std::vector<GameObject*> children;
private:
    std::vector<std::shared_ptr<UIComponent>> _components;
    std::vector<std::shared_ptr<UIComponent>> removes;
    bool isActive = true;
    bool isCreated = false;
};