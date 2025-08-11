#ifndef LIFE_TIME_COMPONENT_H
#define LIFE_TIME_COMPONENT_H

#include "Components/Base/Component.h"

class LifeTimeComponent :public Component
{
public:
    LifeTimeComponent(const std::string& name, std::shared_ptr<Actor> owner) :Component(name, owner) {};
    virtual ~LifeTimeComponent() = default;

    void Initialize()override {};

    // ������ݒ肷��
    void SetLifeTime(float seconds)
    {
        lifeTime_ = seconds;
        elapsedTime_ = 0.0f;
        isStartCountDown_ = true;
    }

    void Tick(float deltaTime) override;

    virtual void UpdateComponentToWorld(UpdateTransformFlags update_transform_flags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None)override
    {
        // LogicComponent�ɂ�Transform���Ȃ��̂ŁA�������Ȃ�
    }

    float GetLifeTime() const { return this->lifeTime_; }
    float GetElapsedTime() const { return elapsedTime_; }
    float GetRemainingTime() const { return lifeTime_ - elapsedTime_; }

private:
    float lifeTime_ = 0.0f; // ����
    float elapsedTime_ = 0.0f;  // �o�ߎ���
    bool isStartCountDown_ = true;

};




#endif // !LIFE_TIME_COMPONENT_H
