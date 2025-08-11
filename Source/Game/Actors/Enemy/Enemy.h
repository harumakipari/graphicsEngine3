#pragma once
#include "Game/Actors/Base/Character.h"

struct ParticleSystem;

enum class MessageType :int
{
    MsgCallHelp,
    MsgChangeAttackRight,
    MsgGiveAttackRight,
    MsgDontGiveAttackRight,
    MsgAskAttackRight,
};

class Telegram
{
public:
    int sender;
    int receiver;
    MessageType msg;

    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
    Telegram(const Telegram&) = delete;
    Telegram& operator=(const Telegram&) = delete;

    Telegram(int sender, int receiver, MessageType msg) :sender(sender), receiver(receiver), msg(msg)
    {
    }
};

class Enemy :public Character
{
public:
    Enemy() = default;
    ~Enemy() override {}

    Enemy(const std::string& modelName) :Character(modelName)
    {
    }



    //�R�s�[�R���X�g���N�^�ƃR�s�[������Z�q���֎~�ɂ���
    Enemy(const Enemy&) = delete;
    Enemy& operator=(const Enemy&) = delete;

    //void Update(float elapsedTime)override;
    //virtual void Update(float elapsedTime, DirectX::XMFLOAT3 playerPosition) = 0;

    virtual void UpdateParticle(ID3D11DeviceContext* immediate_context, float deltaTime, ParticleSystem* p) {};

    virtual bool OnMessage(const Telegram& msg) { return false; }
};
