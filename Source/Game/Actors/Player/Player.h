#pragma once
#include <stack>
#include <memory>
#include "Game/Actors/Base/Character.h"
#include "Engine/Input/GamePad.h"
#include "PlayerState.h"

//TODO :01 ここ変更する
// colliderをセットするときに位置や半径、追加したい当たり判定の種類を伝えてセットできるようにする
//#include "Physics/Collider.h"
//
//#include "Physics/CollisionEvent.h"
//#include "Physics/Physics.h"
//#include "Core/ActorManager.h"


#include "Components/Controller/ControllerComponent.h"
#include "Components/Render/MeshComponent.h"
#include "Components/Effect/EffectComponent.h"

#include "Game/Actors/Item/PickUpItem.h"
#include "Game/Actors/Item/HeldEnergyCore.h"
#include "Game/Actors/Enemy/RiderEnemy.h"
#include "Game/Managers/GameManager.h"
#include "Game/Managers/ItemManager.h"

#include "Core/ActorManager.h"
#include "Components/Audio/AudioSourceComponent.h"



enum class AnimationState
{
    Idle,
    Running,
    Attack,
    Attack_First,
    Jump_Start,
    Jump_Apex,
    Jump_Land,
    Jump_Recovering,
    Jump_Attack,
    Hit_Damaged,
    Emote,
};

class Player :public Character
{
public:
    enum class State
    {
        Idle,
        Running,
        StartCharge,
        FireBeam,
        FinishBeam,
        Attack,
        CantChargeBeam, // チュートリアルの時に使用するステート
        CantMoveCharge,
    };

    // ステートを設定する
    void SetState(Player::State state) { this->state = state; }
private:
    State state = Player::State::CantMoveCharge;
public:
    Player() = default;
    ~Player() = default;

    Player(const std::string& modelName) :Character(modelName)
    {
        // カプセルの当たり判定を生成
        radius = 0.2f;
        height = 0.9f;
        mass = 50.0f;
        hp = maxHp;
        //PushState(std::make_shared<IdlingState>());
    }
    // 描画用コンポーネントを追加
    std::shared_ptr<SkeltalMeshComponent> skeltalMeshComponent;
    // 描画の基準点　左
    std::shared_ptr<SkeltalMeshComponent> leftComponent;
    //std::shared_ptr<SceneComponent> leftComponent;
    std::shared_ptr<BoxComponet> boxLeftHitComponent;
    // 描画の基準点　右
    std::shared_ptr<SkeltalMeshComponent> rightComponent;
    std::shared_ptr<BoxComponet> boxRightHitComponent;

    // ダメージ用の当たり判定
    std::shared_ptr<SphereComponent> playerDamageLeft;
    std::shared_ptr<SphereComponent> playerDamageRight;

    void Initialize(const Transform& transform)override;

    // 入力をオンにするか
    bool CanMove()override
    {
        // チャージビーム状態じゃなかったら入力をオンにする
        return state != State::StartCharge && state != State::CantMoveCharge;
    }

    std::shared_ptr<EffectComponent> effectChargeComponent;
    std::shared_ptr<SphereComponent> sphereRightComponent;
    std::shared_ptr<InputComponent> inputComponent;
    std::shared_ptr<RotationComponent> rotationComponent;
    std::shared_ptr<SphereComponent> sphereLeftComponent;
    std::shared_ptr<MovementComponent> movementComponent;
    std::shared_ptr<AudioSourceComponent> beamChargeAudioComponent;
    std::shared_ptr<AudioSourceComponent> beamLaunchAudioComponent;
    std::shared_ptr<AudioSourceComponent> itemAudioComponent;
    DirectX::XMFLOAT3 GetVelocity() { return velocity; }

    void Update(float elapsedTime)override;

    // 遅延更新処理
    void LateUpdate(float elapsedTime)override;

    //下向きのレイのステージとの交点を取得する関数
    DirectX::XMFLOAT3 GetStageIntersect() const { return intersectStagePosition; }

    void DrawImGuiDetails()override;

    void Finalize()override
    {
        leftItemCount = 0;
        rightItemCount = 0;
    }
private:
    // ビームをチャージする関数
    void TryStartCharge();

    // ビームを発射する関数
    void FireBeam();

    void Move(float elapsedTime)override;

    void Turn(float elapsedTime);
public:
    //スティック入力値から移動ベクトルを取得
    DirectX::XMFLOAT3 GetMoveVec();

    void HandleInput(GamePad& pad);

    void PushState(std::shared_ptr<PlayerState> state);

    void PopState();

    void ChangeState(std::shared_ptr<PlayerState> state);

    //プレイヤーの今のステートを取得する
    std::shared_ptr<PlayerState> GetCurrentState()
    {
        if (!stateStack.empty())
        {
            return stateStack.top();
        }
        return 0;
    }

    //当たった時の処理
    void Hit();

    // 所持アイテムを全てクリアする
    void HasItemReset()
    {
        // 当たり判定の範囲を戻す
        boxLeftHitComponent->ResizeBox(firstHalfBoxExtent.x, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxLeftHitComponent->SetRelativeLocationDirect(firstLeftBoxPosition);
        boxRightHitComponent->ResizeBox(firstHalfBoxExtent.x, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxRightHitComponent->SetRelativeLocationDirect(firstRightBoxPosition);
        // カウントを戻す
        leftItemCount = 0;
        rightItemCount = 0;
        // 見た目の描画のスケールを戻す
        DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
        leftComponent->SetRelativeScaleDirect(leftScale);
        DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
        rightComponent->SetRelativeScaleDirect(rightScale);
        // player のダメージの当たり判定を戻す
        //playerDamageLeft->SetRelativeLocationDirect(firstLeftDamagePosition);
        playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + radius), rightFirstPos.y, rightFirstPos.z));
        playerDamageLeft->ResizeSphere(firstDamageRadius);
        //playerDamageRight->SetRelativeLocationDirect(firstRightDamagePosition);
        playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightFirstPos.x + radius), rightFirstPos.y, rightFirstPos.z));
        playerDamageRight->ResizeSphere(firstDamageRadius);
    }

    // 左のアイテムストックにダメージを適応する
    void ApplyDamageToLeft(int damage)
    {
        if (damage <= 0)
        {
            return;
        }

        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "Before:leftItemCount : %d,PlayerHp : %d\n",
            leftItemCount, hp);
        OutputDebugStringA(debugBuffer);

        // リザルト用::プレイヤーの被弾回数
        GameManager::CallPlayerDamaged();
        BlinkInit();

        int leftDamage = damage;
        int leftRestDamage = leftDamage - leftItemCount;
        leftItemCount -= leftDamage;
        if (leftItemCount < 0)
        {
            leftItemCount = 0;
        }
        if (leftRestDamage > 0)
        {
            this->hp -= leftRestDamage;
        }
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "After:leftItemCount : %d,PlayerHp : %d\n",
            leftItemCount, hp);
        OutputDebugStringA(debugBuffer);

        //int leftDamage = std::min<int>(damage, leftItemCount);
        //leftItemCount -= leftDamage;
        //damage -= leftDamage;
        //if (damage > 0)
        //{// 残りは本体に適応
        //    this->hp -= damage;
        //}
        // 当たり判定を小さくする


        DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
        leftScale.x += leftItemCount * scaleBigSize;
        leftScale.y += leftItemCount * scaleBigSize;
        leftScale.z += leftItemCount * scaleBigSize;
        // プレイヤーの左のアイテムの収集の当たり判定を大きくする
        float leftBoxWidth = -leftFirstPos.x + leftScale.x * firstPlayerSideSize;
        boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, firstLeftBoxPosition.y, firstLeftBoxPosition.z));
        // プレイヤーの左のダメージ当たり判定を大きくする
        float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player の半径は足さない
        playerDamageLeft->ResizeSphere(leftDamageRadius);
        playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + leftDamageRadius), rightFirstPos.y + leftDamageRadius, rightFirstPos.z));
        // スケールを小さくする処理
        afterLeftScale = leftScale.x;
        isLeftShrinking = true;
        shrinkElapsedTimeLeft = 0.0f;
        currentLeftScale = leftComponent->GetRelativeScale().x;
    }

    // 右のアイテムストックにダメージを適応する
    void ApplyDamageToRight(int damage)
    {
        if (damage <= 0)
        {
            return;
        }
        char debugBuffer[128];
        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "Before:rightItemCount : %d,PlayerHp : %d\n",
            rightItemCount, hp);
        OutputDebugStringA(debugBuffer);

        // リザルト用::プレイヤーの被弾回数
        GameManager::CallPlayerDamaged();
        BlinkInit();
        int rightDamage = damage;
        int rightRestDamage = rightDamage - rightItemCount;
        rightItemCount -= rightDamage;
        if (rightItemCount < 0)
        {
            rightItemCount = 0;
        }
        if (rightRestDamage > 0)
        {
            this->hp -= rightRestDamage;
        }

        sprintf_s(debugBuffer, sizeof(debugBuffer),
            "After:rightItemCount : %d,PlayerHp : %d\n",
            rightItemCount, hp);
        OutputDebugStringA(debugBuffer);

        //int rightDamage = std::min<int>(damage, rightItemCount);
        //damage -= rightDamage;

        //if (damage > 0)
        //{// 残りは本体に適応
        //    this->hp -= damage;
        //}

        // 当たり判定を小さくする
        DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
        rightScale.x += rightItemCount * scaleBigSize;
        rightScale.y += rightItemCount * scaleBigSize;
        rightScale.z += rightItemCount * scaleBigSize;
        // プレイヤーの右のアイテムの収集の当たり判定を大きくする
        float rightBoxWidth = rightFirstPos.x + rightScale.x * firstPlayerSideSize;
        boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
        boxRightHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightBoxWidth) * 0.5f, firstRightBoxPosition.y, firstRightBoxPosition.z));
        // プレイヤーの右のダメージ当たり判定を大きくする
        float rightDamageRadius = (rightScale.x * firstPlayerSideSize) * 0.5f; // player の半径は足さない
        playerDamageRight->ResizeSphere(rightDamageRadius);
        playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3(rightFirstPos.x + rightDamageRadius, rightFirstPos.y + rightDamageRadius, rightFirstPos.z));


        // スケールを小さくする処理
        isRightShrinking = true;
        shrinkElapsedTimeRight = 0.0f;
        currentRightScale = rightComponent->GetRelativeScale().x;
        afterRightScale = rightScale.x;
    }

    // プレイヤーの本体に直接ダメージ処理
    void ApplyDirectHpDamage(int damage)
    {
        this->hp -= damage;
    }

    // スケールを戻す処理
    void UpdateItemVisualShrink(float deltaTime)
    {
        //// 一秒で戻る
        //const float shrinkDuration = 1.0f;

        //if (isLeftShrinking)
        //{// 左の処理
        //    shrinkElapsedTimeLeft += deltaTime;
        //    float t = std::min<float>(shrinkElapsedTimeLeft / shrinkDuration, 1.0f);
        //    float scale = std::lerp(currentLeftScale, afterLeftScale, t);
        //    DirectX::XMFLOAT3 newScale = { scale,scale,scale };
        //    leftComponent->SetRelativeScaleDirect(newScale);

        //    if (t >= 1.0f)
        //    {
        //        isLeftShrinking = false;
        //    }
        //}

        //if (isRightShrinking)
        //{// 右に当たったら
        //    shrinkElapsedTimeRight += deltaTime;
        //    float t = std::min<float>(shrinkElapsedTimeRight / shrinkDuration, 1.0f);
        //    float scale = std::lerp(currentRightScale, afterRightScale, t);
        //    DirectX::XMFLOAT3 newScale = { scale,scale,scale };
        //    rightComponent->SetRelativeScaleDirect(newScale);

        //    if (t >= 0.0f)
        //    {
        //        isRightShrinking = false;
        //    }
        //}
    }


public:
    //上方向への力
    float jumpPower = 5.0f;

    //武器のノード番号
    //size_t nodeAttackIndex = 153; //"VB root Weapon"
    size_t nodeAttackIndex = 0; //"VB root Weapon"

    // 左のアイテム保有数を取得する関数
    int GetLeftItemCount() const { return leftItemCount; }
    // 右のアイテム保有数を取得する関数
    int GetRightItemCount() const { return rightItemCount; }

    void ResetHitFlags()
    {
        hitLeftThisFrame = false;
        hitRightThisFrame = false;
        currentFrameDamage = 0;
        hasDamageThisFrame = false;
    }
private:
    GamePad pad;
    //頭の天辺のノード番号
    size_t nodeTopIndex = 126;   //"hair_top_mid_01"
    //頭の下のノード番号
    size_t nodeBottomIndex = 146;    //"ik_foot_root"
    //プレイヤーのステート
    std::stack<std::shared_ptr<PlayerState>> stateStack;

    // プレイヤーが所持しているアイテム
    //std::vector<std::shared_ptr<HeldEnergyCore>> hasLeftItems;
    //std::vector<std::shared_ptr<HeldEnergyCore>> hasRightItems;
    // 左側のアイテム数
    int leftItemCount = 0;
    // 右側のアイテム数
    int rightItemCount = 0;
    // player Side の直径の初期値
    float firstPlayerSideSize = 0.35f;
    //float firstPlayerSideSize = 0.5f;
    // どれくらい player side が大きくなっていくか
    float scaleBigSize = 0.2f;
    // 最初の左右の当たり判定の大きさ
    //DirectX::XMFLOAT3 firstHalfBoxExtent = { 0.2f,0.45f,0.2f };
    DirectX::XMFLOAT3 firstHalfBoxExtent = { 0.5f,0.45f,0.7f };
    // 最初の左右のアイテムの収集の当たり判定の位置
    DirectX::XMFLOAT3 firstLeftBoxPosition = { -0.5f , -0.4f, 0.5f };
    DirectX::XMFLOAT3 firstRightBoxPosition = { 0.5f , -0.4f, 0.5f };
    //DirectX::XMFLOAT3 firstLeftBoxPosition = { -radius , 0.0f, 0.0f };
    //DirectX::XMFLOAT3 firstRightBoxPosition = { radius , 0.0f, 0.0f };
    // 最初の左右のダメージの当たり判定の位置
    DirectX::XMFLOAT3 firstLeftDamagePosition = { -radius , 0.0f, 0.0f };
    DirectX::XMFLOAT3 firstRightDamagePosition = { radius , 0.0f, 0.0f };
    float firstDamageRadius = 0.01f;
    // アイテムの半径
    float itemRadius = 0.5f;
    // アイテムが減った時に左のタンクを収縮するフラグ
    bool isLeftShrinking = false;
    // アイテムが減った時に左のタンクを収縮する経過時間
    float shrinkElapsedTimeLeft = 0.0f;
    // アイテムが減る前のスケール
    float currentLeftScale = 0.0f;
    // アイテムが減ったあと前のスケール
    float afterLeftScale = 0.0f;
    // アイテムが減った時に右のタンクを収縮するフラグ
    bool isRightShrinking = false;
    // アイテムが減った時に右のタンクを収縮する経過時間
    float shrinkElapsedTimeRight = 0.0f;
    // アイテムが減る前のスケール
    float currentRightScale = 0.0f;
    // アイテムが減ったあと前のスケール
    float afterRightScale = 0.0f;
    // プレイヤーの eraseInArea 無敵時間
    float invisibleTime = 0.0f;
    // プレイヤーの boss 無敵時間
    float bossInvisibleTime = 0.0f;
    // プレイヤーの現在のスピード
    float currentSpeed = 5.0f;
    // プレイヤーはアイテムを持っていないときのスピード
    float noItemSpeed = 10.0f;
    // プレイヤーの Max スピード
    float maxSpeed = 5.0f;
    // プレイヤーの Min スピード
    float minSpeed = 2.0f;
    // プレイヤーの現在の回転スピード
    float currentTurnSpeed = 720.0f;
    // プレイヤーの Max 回転スピード
    float maxTurnSpeed = 720.0f;
    // プレイヤーの Min 回転スピード
    float minTurnSpeed = 90.0f;
    // 既に回収されたアイテム化を確認する
    PickUpItem* lastHitPickUpItem = nullptr;
    // プレイヤーのマックスHP
    int maxHp = 20;
    bool isIdleEnd = false;
public:
    // 無敵時間中かどうか
    bool IsInvincible() const { return invisibleTime > 0.0f; }
    // 無敵時間間隔設定
    void SetInvincible(float maxInvincibleTime = 3.0f) { invisibleTime = maxInvincibleTime; }

    // 無敵時間中かどうか
    bool IsBossInvincible() const { return bossInvisibleTime > 0.0f; }
    // 無敵時間間隔設定
    void SetBossInvincible(float maxInvincibleTime = 3.0f) { bossInvisibleTime = maxInvincibleTime; }

    // EraseInArea で使用
    // 次のフレームで適応する無敵時間のフラグを立てる
    bool applyInvincibilityNextFrame = false;
    // 初回だけダメージを換算したいから
    bool hasDamageThisFrame = false;
    // ダメージ記録
    bool hitLeftThisFrame = false;
    bool hitRightThisFrame = false;
    int currentFrameDamage = 0;

    DirectX::XMFLOAT3 angle = { 0.0f,0.0f,0.0f };
    DirectX::XMFLOAT3 prePosition = { 0.0f,0.0f,0.0f };

    int leftItemMax = 15;
    int rightItemMax = 15;

    // 初回のサイズ
    DirectX::XMFLOAT3 leftFirstPos = { -0.5f,-0.5f,0.2f };
    DirectX::XMFLOAT3 rightFirstPos = { 0.5f,-0.5f,0.2f };

private:
    // プレイヤー被弾時の点滅
    float hitBlinkElapsed = 0.0f;
    float hitBlinkInterval = 0.1f;
    float hitBlinkTotalTime = 1.5f;
    bool isHitBlinking = false;
    bool isRed = false;

    void BlinkInit()
    {
        isHitBlinking = true;
        hitBlinkElapsed = 0.0f;
    }

    void SetBlinkColor(bool isRed)
    {
        if (isRed)
        {
            color = { 1.0f,0.2f,0.2f };
        }
        else
        {
            color = { 1.0f,1.0f,1.0f };
        }
    }

    DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };

    // 
    bool onceFrag = false;

    // 左右の色
    DirectX::XMFLOAT3 firstColor = { 0.302f,0.910f,1.0f };
    DirectX::XMFLOAT3 secondColor = { 0.0f, 0.85f, 0.55f };
    DirectX::XMFLOAT3 thirdColor = { 0.0f, 1.0f, 0.4f };
    DirectX::XMFLOAT3 FourthColor = { 0.5f, 0.4f, 1.0f };
    DirectX::XMFLOAT3 FinalColor = { 1.0f,0.239f,1.0f };

    // 左右のアイテム数によって色を変更する
    void ItemColor(float deltaTime)
    {
        elapsedTime_ += deltaTime;

        // 線形補間関数
        auto LerpColor = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t) -> DirectX::XMFLOAT3
            {
                t = std::clamp(t, 0.0f, 1.0f);
                return {
                    std::lerp(a.x, b.x, t),
                    std::lerp(a.y, b.y, t),
                    std::lerp(a.z, b.z, t)
                };
            };

        static const std::vector<std::pair<int, DirectX::XMFLOAT3>> colorStops = {
            {0,  {0.302f, 0.910f, 1.0f}},   
            {4,  {0.0f,   0.85f,  0.55f}},  
            {7,  {0.0f,   1.0f,   0.4f}},   
            {10, {0.5f,   0.4f,   1.0f}},   
            {13, {1.0f,   0.239f, 1.0f}},   
            {15, {1.0f,   0.239f, 1.0f}},   
        };

        auto GetColor = [&](int count) -> DirectX::XMFLOAT3
            {
                // 上限
                if (count >= colorStops.back().first)
                    return colorStops.back().second;

                for (size_t i = 0; i < colorStops.size() - 1; ++i)
                {
                    auto [startCount, startColor] = colorStops[i];
                    auto [endCount, endColor] = colorStops[i + 1];

                    if (count >= startCount && count < endCount)
                    {
                        float t = static_cast<float>(count - startCount) / (endCount - startCount);
                        return LerpColor(startColor, endColor, t);
                    }
                }

                return colorStops.front().second; 
            };

        leftComponent->model->cpuColor = GetColor(leftItemCount);
        rightComponent->model->cpuColor = GetColor(rightItemCount);
    }


    // MAXアイテムの時に色がコロコロ変わる
    DirectX::XMFLOAT3 MaxColorChange(float h, float s, float v)
    {
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h * 6.0f, 2.0f) - 1));
        float m = v - c;

        float r, g, b;
        if (h < 1.0f / 6.0f) { r = c; g = x; b = 0; }
        else if (h < 2.0f / 6.0f) { r = x; g = c; b = 0; }
        else if (h < 3.0f / 6.0f) { r = 0; g = c; b = x; }
        else if (h < 4.0f / 6.0f) { r = 0; g = x; b = c; }
        else if (h < 5.0f / 6.0f) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }

        return { r + m, g + m, b + m };
    }
private:
    float elapsedTime_ = 0.0f;
};