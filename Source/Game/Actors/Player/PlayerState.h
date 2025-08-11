#pragma once
#include <memory>
#include <string>
class Player;
class GamePad;


class PlayerState
{
public :
    virtual ~PlayerState() {}
    virtual void HandleInput(Player& player, GamePad& input){}
    virtual void Update(Player& player){}
    virtual void Enter(Player& player){}
    virtual void Exit(Player& player){}

    virtual std::string GetStateName() const = 0; // �X�e�[�g�����擾
};


class IdlingState :public PlayerState
{
public:
    IdlingState(){}

    void Update(Player& player)override;

    void HandleInput(Player& player, GamePad& input)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "IdlingState"; }
};

class RunningState :public PlayerState
{
public:
    RunningState() {}

    void Update(Player& player)override;

    void HandleInput(Player& player, GamePad& input)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "RunningState"; }

};

class AttackState :public PlayerState
{
public:
    AttackState() {}

    void HandleInput(Player& player, GamePad& input)override;

    void Update(Player& player)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "AttackState"; }

};

//�W�����v�J�n
class JumpStartState :public PlayerState
{
public:
    JumpStartState(){}

    void HandleInput(Player& player, GamePad& input)override;

    void Update(Player& player)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "JumpStartState"; }

private:
};

//�W�����v�r��
class JumpMidState :public PlayerState
{
public:
    JumpMidState() {}

    void HandleInput(Player& player, GamePad& input)override;

    void Update(Player& player)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "JumpMidState"; }

};

//�W�����v�I���
class JumpFinishState :public PlayerState
{
public:
    JumpFinishState() {}

    void HandleInput(Player& player, GamePad& input)override;

    void Update(Player& player)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "JumpFinishState"; }

private:
    bool isAnimatedReconvering = false;
};

//�W�����v�̒��n
class JumpRecoveringState :public PlayerState
{
public:
    JumpRecoveringState() {}

    void HandleInput(Player& player, GamePad& input)override;

    void Update(Player& player)override;

    void Enter(Player& player)override;

    void Exit(Player& player)override;

    std::string GetStateName() const override { return "JumpRecoveringState"; }
};
