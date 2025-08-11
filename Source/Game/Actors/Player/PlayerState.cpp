#include "PlayerState.h"
#include "Engine/Input/GamePad.h"
#include "Player.h"

//�ҋ@
//�X�V����
void IdlingState::Update(Player& player)
{

}

void IdlingState::HandleInput(Player& player, GamePad& input)
{
    if (std::fabs(input.ThumbStateLx()) > 0.1f || std::fabs(input.ThumbStateLy()) > 0.1f)
    {
        player.ChangeState(std::make_shared<RunningState>());
    }
    //if (input.ButtonState(GamePad::Button::y, TriggerMode::risingEdge))
    if (input.ButtonState(GamePad::Button::y, TriggerMode::none))
    {//[v]
        player.ChangeState(std::make_shared<JumpStartState>());
    }
    //if (input.ButtonState(GamePad::Button::x, TriggerMode::risingEdge))
    if (input.ButtonState(GamePad::Button::x, TriggerMode::none))
    {
        player.PushState(std::make_shared<AttackState>());
    }
}

void IdlingState::Enter(Player& player)
{
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Idle), true);
}

void IdlingState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Idle), 0.1f);
}

//����
void RunningState::Update(Player& player)
{

}

void RunningState::HandleInput(Player& player, GamePad& input)
{
    if (std::fabs(input.ThumbStateLx()) < 0.1f && std::fabs(input.ThumbStateLy()) < 0.1f)
    {
        player.ChangeState(std::make_shared<IdlingState>());
    }
    if (input.ButtonState(GamePad::Button::x, TriggerMode::none))
    {
        player.PushState(std::make_shared<AttackState>());
    }
    if (input.ButtonState(GamePad::Button::y, TriggerMode::none))
    {//[v]
        player.ChangeState(std::make_shared<JumpStartState>());
    }
}

void RunningState::Enter(Player& player)
{
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Running), true);
}

void RunningState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Running), 0.1f);
}

//�U��
void AttackState::HandleInput(Player& player, GamePad& input)
{
    //if (player.GetModelComponent().isAnimationFinished)
    {//�A�j���[�V�������I�������
        if (std::fabs(input.ThumbStateLx()) > 0.1f || std::fabs(input.ThumbStateLy()) > 0.1f)
        {
            player.ChangeState(std::make_shared<RunningState>());
        }
    }
}

void AttackState::Update(Player& player)
{
    using namespace DirectX;

    //�U���̎��v���C���[�̑O���ɓ����蔻�������
    //DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&player.GetForward());
    //Forward = DirectX::XMVector3Normalize(Forward);
    //DirectX::XMVECTOR playerPos = XMLoadFloat3(&player.GetPosition());
    //DirectX::XMVECTOR offset = Forward * 1.0f;
    //DirectX::XMVECTOR attackPosVec = /*playerPos*/ + offset;
    //DirectX::XMFLOAT3 attackPosition;
    //XMStoreFloat3(&attackPosition, attackPosVec);
    //attackPosition.y = player.GetPosition().y + 0.5f;

    //if (player.GetModelComponent().isAnimationFinished)
    {//�A�j���[�V�������I�������
        player.PopState();
    }
}

void AttackState::Enter(Player& player)
{
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Attack_First), false);
}

void AttackState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Attack_First), 0.1f);
}

//�W�����v�J�n
void JumpStartState::HandleInput(Player& player, GamePad& input)
{

}

void JumpStartState::Update(Player& player)
{
    //if (player.velocity.y <= 0)
    //�㏸������������
    //if (player.GetModelComponent().isAnimationFinished)
    {//�A�j���[�V�����̍Đ����I������� 
        //player.ChangeState(std::make_shared<JumpMidState>());
        player.ChangeState(std::make_shared<JumpFinishState>());
    }
}

void JumpStartState::Enter(Player& player)
{
    //�W�����v�̏����x
    player.velocity.y = player.jumpPower;

    //�㏸���ԁ@�@v = v0 + gt ��ό`���� �W�����v�̏㏸�� v �� 0 �ɂȂ邩�� t = v0 / g ;
    float t_up = player.jumpPower / std::fabs(player.gravity);

    //Jump_Start�̃A�j���[�V��������
    //float t_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Start));

    //�A�j���[�V�����̍Đ����x��t_up�ɍ��킹��
    //float animationRate = t_anim / t_up;
    //player.GetModelComponent().SetAnimationRate(animationRate);

    //Jump_Start����x�Đ�
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Start), false);
}

void JumpStartState::Exit(Player& player)
{
    //�A�j���[�V�����̍Đ����x��߂��Ă���
    //player.GetModelComponent().SetAnimationRate(1.0f);

    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Start), 0.1f);
}

//�W�����v�r��
void JumpMidState::HandleInput(Player& player, GamePad& input)
{

}

void JumpMidState::Update(Player& player)
{
    //if (player.isFalling())
    //if (player.GetModelComponent().isAnimationFinished /*&& player.velocity.y < 0.0f*/)
    {//�㏸������������

        player.ChangeState(std::make_shared<JumpFinishState>());
    }
}

void JumpMidState::Enter(Player& player)
{
    //"Jump_Middle"��1�x�Đ�
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Apex), false);
}

void JumpMidState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Apex), 0.05f);
}

//�W�����v�I���
void JumpFinishState::HandleInput(Player& player, GamePad& input)
{

}

void JumpFinishState::Update(Player& player)
{
    if (player.isGround && !isAnimatedReconvering)
    {//�n�ʂɂ�����
        player.ChangeState(std::make_shared<JumpRecoveringState>());
    }
}

void JumpFinishState::Enter(Player& player)
{
    isAnimatedReconvering = false;
    //TODO:01�����̍l�����m�肽��

    //float t_middle_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Apex));

    //���R�����̌������
    //v0 = �� (2gh)  ����@ h = v0 * v0 / 2g 
    //float h = player.jumpPower * player.jumpPower / (2 * player.gravity);
    float h = player.GetPosition().y - player.GetStageIntersect().y + 1.0f/*�����l*/;
    // t = �� (2h / g) ����@��̎�������ā@
    float t_fall = std::sqrtf(2 * h / std::abs(player.gravity)) /* - t_middle_anim*//*"Jump_Middle"*/;


    //"Jump_Finish"�̃A�j���[�V�����̍Đ����Ԏ擾
    //float t_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Land));

    //�A�j���[�V�����̍Đ����x��"t_fall"�ɍ��킹��
    //float animationRate = t_anim / t_fall;
    //player.GetModelComponent().SetAnimationRate(animationRate);

    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Land), false);
}

void JumpFinishState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Land), 0.05f);
}

//�W�����v���n
void JumpRecoveringState::HandleInput(Player& player, GamePad& input)
{

}

void JumpRecoveringState::Update(Player& player)
{
    //if (player.GetModelComponent().isAnimationFinished)
    {
        player.ChangeState(std::make_shared<IdlingState>());
    }
}

void JumpRecoveringState::Enter(Player& player)
{
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Recovering), false);
}

void JumpRecoveringState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Recovering), 0.05f);

    //�u�����h����Ƙr����ɏオ�邩��u�����h�Ȃ���
    //player.GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Idle));
}
