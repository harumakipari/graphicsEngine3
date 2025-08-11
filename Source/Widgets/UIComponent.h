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

	GameObject* gameObject = nullptr; //Component����������GameObject
	RectTransform* rect = nullptr;
	void SetEnable(bool set);
	bool IsEnable() const { return enable; }
	void Destroy();

private:
	friend class GameObject;
	//��ԍŏ��̏���������
	virtual void Awake() {};
	//����������
	virtual void Initialize() {};
	//�X�V����
	virtual void Update(float elapsedTime) {};
	//2D�`��J�n�O����
	virtual void Begin(ID3D11DeviceContext* immediateContext) {};
	//2D�`�揈��
	virtual void Draw(ID3D11DeviceContext* immediateContext) {};
	//2D�`��I���㏈��
	virtual void End(ID3D11DeviceContext* immediateContext) {};
	//�f�o�b�OGUI�`��
	virtual void DrawProperty() {};
	//�I��������
	virtual void Finalize() {};

	virtual void OnEnable() {};
	virtual void OnDisable() {};

	void SetOwner(GameObject* gameObject) { this->gameObject = gameObject; }
	void SetName(const std::string& name) { this->name = name; }

	bool enable = true;
public:
	bool hideInspector = false;//Inspector���炱�̃R���|�[�l���g���B�����ǂ���
	bool hideInspecterProperty = false;//Inspector�ŁA���̗v�f���B�����ǂ���
public:
	//bool isRaycastTarget = false;
	std::string name;
};