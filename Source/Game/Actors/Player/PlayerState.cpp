#include "PlayerState.h"
#include "Engine/Input/GamePad.h"
#include "Player.h"

//待機
//更新処理
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

//走り
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

//攻撃
void AttackState::HandleInput(Player& player, GamePad& input)
{
    //if (player.GetModelComponent().isAnimationFinished)
    {//アニメーションが終わったら
        if (std::fabs(input.ThumbStateLx()) > 0.1f || std::fabs(input.ThumbStateLy()) > 0.1f)
        {
            player.ChangeState(std::make_shared<RunningState>());
        }
    }
}

void AttackState::Update(Player& player)
{
    using namespace DirectX;

    //攻撃の時プレイヤーの前方に当たり判定をつける
    //DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&player.GetForward());
    //Forward = DirectX::XMVector3Normalize(Forward);
    //DirectX::XMVECTOR playerPos = XMLoadFloat3(&player.GetPosition());
    //DirectX::XMVECTOR offset = Forward * 1.0f;
    //DirectX::XMVECTOR attackPosVec = /*playerPos*/ + offset;
    //DirectX::XMFLOAT3 attackPosition;
    //XMStoreFloat3(&attackPosition, attackPosVec);
    //attackPosition.y = player.GetPosition().y + 0.5f;

    //if (player.GetModelComponent().isAnimationFinished)
    {//アニメーションが終わったら
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

//ジャンプ開始
void JumpStartState::HandleInput(Player& player, GamePad& input)
{

}

void JumpStartState::Update(Player& player)
{
    //if (player.velocity.y <= 0)
    //上昇が完了した時
    //if (player.GetModelComponent().isAnimationFinished)
    {//アニメーションの再生が終わったら 
        //player.ChangeState(std::make_shared<JumpMidState>());
        player.ChangeState(std::make_shared<JumpFinishState>());
    }
}

void JumpStartState::Enter(Player& player)
{
    //ジャンプの初速度
    player.velocity.y = player.jumpPower;

    //上昇時間　　v = v0 + gt を変形して ジャンプの上昇時 v は 0 になるから t = v0 / g ;
    float t_up = player.jumpPower / std::fabs(player.gravity);

    //Jump_Startのアニメーション時間
    //float t_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Start));

    //アニメーションの再生速度をt_upに合わせる
    //float animationRate = t_anim / t_up;
    //player.GetModelComponent().SetAnimationRate(animationRate);

    //Jump_Startを一度再生
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Start), false);
}

void JumpStartState::Exit(Player& player)
{
    //アニメーションの再生速度を戻しておく
    //player.GetModelComponent().SetAnimationRate(1.0f);

    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Start), 0.1f);
}

//ジャンプ途中
void JumpMidState::HandleInput(Player& player, GamePad& input)
{

}

void JumpMidState::Update(Player& player)
{
    //if (player.isFalling())
    //if (player.GetModelComponent().isAnimationFinished /*&& player.velocity.y < 0.0f*/)
    {//上昇が完了した時

        player.ChangeState(std::make_shared<JumpFinishState>());
    }
}

void JumpMidState::Enter(Player& player)
{
    //"Jump_Middle"を1度再生
    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Apex), false);
}

void JumpMidState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Apex), 0.05f);
}

//ジャンプ終わり
void JumpFinishState::HandleInput(Player& player, GamePad& input)
{

}

void JumpFinishState::Update(Player& player)
{
    if (player.isGround && !isAnimatedReconvering)
    {//地面についたら
        player.ChangeState(std::make_shared<JumpRecoveringState>());
    }
}

void JumpFinishState::Enter(Player& player)
{
    isAnimatedReconvering = false;
    //TODO:01ここの考え方知りたい

    //float t_middle_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Apex));

    //自由落下の公式より
    //v0 = √ (2gh)  から　 h = v0 * v0 / 2g 
    //float h = player.jumpPower * player.jumpPower / (2 * player.gravity);
    float h = player.GetPosition().y - player.GetStageIntersect().y + 1.0f/*調整値*/;
    // t = √ (2h / g) から　上の式代入して　
    float t_fall = std::sqrtf(2 * h / std::abs(player.gravity)) /* - t_middle_anim*//*"Jump_Middle"*/;


    //"Jump_Finish"のアニメーションの再生時間取得
    //float t_anim = player.GetModelComponent().GetAnimationDuration(static_cast<int>(AnimationState::Jump_Land));

    //アニメーションの再生速度を"t_fall"に合わせる
    //float animationRate = t_anim / t_fall;
    //player.GetModelComponent().SetAnimationRate(animationRate);

    //player.GetModelComponent().SetToAnimation(static_cast<int>(AnimationState::Jump_Land), false);
}

void JumpFinishState::Exit(Player& player)
{
    //player.GetModelComponent().SetFromAnimation(static_cast<int>(AnimationState::Jump_Land), 0.05f);
}

//ジャンプ着地
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

    //ブレンドすると腕が上に上がるからブレンドなしで
    //player.GetModelComponent().SetAnimationClip(static_cast<int>(AnimationState::Idle));
}
