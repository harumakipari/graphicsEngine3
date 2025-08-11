#include "Player.h"
#include <float.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "Graphics/Core/Graphics.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsUtility.h"
#include "Core/ActorManager.h"
#include "Game/Actors/Beam/Beam.h"
#include "Game/Actors/Enemy/RiderEnemy.h"

// UIで追加
#include "Widgets/ObjectManager.h"
#include "Widgets/GameObject.h"
#include "Widgets/Mask.h"

// チュートリアルに使用
#include "Game/Managers/TutorialSystem.h"

void Player::Initialize(const Transform& transform)
{
    // 描画用コンポーネントを追加
    skeltalMeshComponent = this->NewSceneComponent<class SkeltalMeshComponent>("skeltalComponent");
    //skeltalMeshComponent->SetModel("./Data/Models/Characters/Player/karichara.gltf");

    skeltalMeshComponent->SetModel("./Data/Models/Characters/Player/chara_animation.gltf");
    //CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    skeltalMeshComponent->model->emission = 5.0f;
    //skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss.gltf");
    //skeltalMeshComponent->SetModel("./Data/Models/Characters/Enemy/boss_idle.gltf");
    //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_emission2");
    //skeltalMeshComponent->SetMaterialPS("./Shader/GltfModelEmissionPS.cso", "L_boss_emission");
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelGameCharacter.cso", skeltalMeshComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());

    SetPosition(transform.GetLocation());
    SetQuaternionRotation(transform.GetRotation());
    SetScale(transform.GetScale());
    //SetScale(DirectX::XMFLOAT3(0.5f,0.5f,0.5f));
    const std::vector<std::string> animationFilenames =
    {
        //"./Data/Models/Characters/Player/result_win.gltf",
        //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Ability_E_InMotion.glb",
        //"..\\glTF-Sample-Models-main\\original\\CharacterAnimation\\Primary_Attack_Fast_A.glb",
    };
    skeltalMeshComponent->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::LH_Y_UP;
    //skeltalMeshComponent->AppendAnimations(animationFilenames);
    // アニメーションコントローラーを作成
    auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
    controller->AddAnimation("Idle", 0);
    //controller->AddAnimation("Win", 1);
    // アニメーションコントローラーをcharacterに追加
    this->SetAnimationController(controller);
    //PlayAnimation("Idle",true,false);

    //leftFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("R_core_FK", skeltalMeshComponent->model->nodes);
    //rightFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("L_core_FK", skeltalMeshComponent->model->nodes);


    // プレイヤーの左の見た目
    leftComponent = this->NewSceneComponent<class SkeltalMeshComponent>("leftComponent", "skeltalComponent");
    leftComponent->SetModel("./Data/Models/Characters/Player/PlayerSide/player_side_left.gltf");
    leftComponent->SetRelativeLocationDirect(leftFirstPos);
    //CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", leftComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPlayerSidePS.cso", leftComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    leftComponent->model->emission = 0.0f;
    leftComponent->model->cpuColor = { 1.0f,1.0f,1.0f };
    // 敵の攻撃が当たる左側 
    playerDamageLeft = this->NewSceneComponent<class SphereComponent>("playerDamageLeft", "skeltalComponent");
    playerDamageLeft->SetRadius(0.01f);
    //playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(radius), 0.0f, 0.0f));
    playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + radius), rightFirstPos.y, rightFirstPos.z));
    playerDamageLeft->SetMass(10.0f);
    playerDamageLeft->SetLayer(CollisionLayer::Player);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::EnemyHand, CollisionComponent::CollisionResponse::Block);
    playerDamageLeft->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    playerDamageLeft->Initialize();
    playerDamageLeft->SetIsVisibleDebugBox(false);
    playerDamageLeft->SetIsVisibleDebugShape(false);

    // 左側のアイテム収集用当たり判定
    boxLeftHitComponent = this->NewSceneComponent<class BoxComponet>("boxHitLeftComponent", "skeltalComponent");
    boxLeftHitComponent->SetHalfBoxExtent(firstHalfBoxExtent);
    boxLeftHitComponent->SetMass(10.0f);
    //boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-radius * 0.5f, 0.0f, 0.0f));
    boxLeftHitComponent->SetRelativeLocationDirect(firstLeftBoxPosition);
    boxLeftHitComponent->SetLayer(CollisionLayer::PlayerSide);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::None);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Trigger);
    //boxLeftHitComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    //boxLeftHitComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);
    boxLeftHitComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    boxLeftHitComponent->SetModelHeight(firstHalfBoxExtent.y);
    boxLeftHitComponent->Initialize();
    //boxLeftHitComponent->SetIsVisibleDebugBox(false);
    //boxLeftHitComponent->SetIsVisibleDebugShape(false);

    // プレイヤーの右の見た目
    rightComponent = this->NewSceneComponent<class SkeltalMeshComponent>("rightComponent", "skeltalComponent");
    rightComponent->SetModel("./Data/Models/Characters/Player/PlayerSide/player_side_right.gltf");
    rightComponent->SetRelativeLocationDirect(rightFirstPos);
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPlayerSidePS.cso", rightComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    rightComponent->model->emission = 0.0f;
    leftComponent->model->cpuColor = { 1.0f,1.0f,1.0f };
    // 敵の攻撃が当たる右側 
    playerDamageRight = this->NewSceneComponent<class SphereComponent>("playerDamageRight", "skeltalComponent");
    playerDamageRight->SetRadius(0.01f);
    //playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3((radius), 0.0f, 0.0f));
    playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3(rightFirstPos.x + radius, rightFirstPos.y, rightFirstPos.z));
    playerDamageRight->SetMass(10.0f);
    playerDamageRight->SetLayer(CollisionLayer::Player);
    playerDamageRight->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    playerDamageRight->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    playerDamageRight->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
    playerDamageRight->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    playerDamageRight->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    playerDamageRight->SetResponseToLayer(CollisionLayer::EnemyHand, CollisionComponent::CollisionResponse::Block);
    playerDamageRight->Initialize();
    playerDamageRight->SetIsVisibleDebugBox(false);
    playerDamageRight->SetIsVisibleDebugShape(false);

    // 右側のアイテム収集用当たり判定
    boxRightHitComponent = this->NewSceneComponent<class BoxComponet>("boxHitRightComponent", "skeltalComponent");
    boxRightHitComponent->SetHalfBoxExtent(firstHalfBoxExtent);
    boxRightHitComponent->SetMass(10.0f);
    boxRightHitComponent->SetRelativeLocationDirect(firstRightBoxPosition);
    boxRightHitComponent->SetLayer(CollisionLayer::Player);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::None);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::PickUpItem, CollisionComponent::CollisionResponse::Trigger);
    //boxRightHitComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    //boxRightHitComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::Block);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::Block);
    boxRightHitComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::Block);
    boxRightHitComponent->SetModelHeight(firstHalfBoxExtent.y);
    boxRightHitComponent->Initialize();
    //boxRightHitComponent->SetIsVisibleDebugBox(false);
    //boxRightHitComponent->SetIsVisibleDebugShape(false);

    //// 敵からの攻撃を受ける当たり判定用のコンポーネントを追加
    std::shared_ptr<CapsuleComponent> capsuleComponent = this->NewSceneComponent<class CapsuleComponent>("capsuleComponent", "skeltalComponent");
    capsuleComponent->SetRadiusAndHeight(radius, height);
    capsuleComponent->SetMass(mass);
    capsuleComponent->SetCapsuleAxis(ShapeComponent::CapsuleAxis::y);
    capsuleComponent->SetRelativeEulerRotationDirect(DirectX::XMFLOAT3(90.0f, 0.0f, 0.0f));
    capsuleComponent->SetLayer(CollisionLayer::Player);
    capsuleComponent->SetResponseToLayer(CollisionLayer::ShockWave, CollisionComponent::CollisionResponse::None);
    capsuleComponent->SetResponseToLayer(CollisionLayer::PlayerSide, CollisionComponent::CollisionResponse::None);
    capsuleComponent->SetResponseToLayer(CollisionLayer::EraseInArea, CollisionComponent::CollisionResponse::Trigger);
    capsuleComponent->SetResponseToLayer(CollisionLayer::Enemy, CollisionComponent::CollisionResponse::Block);
    capsuleComponent->SetResponseToLayer(CollisionLayer::WorldStatic, CollisionComponent::CollisionResponse::None);
    //capsuleComponent->SetResponseToLayer(CollisionLayer::Building, CollisionComponent::CollisionResponse::None);
    capsuleComponent->SetResponseToLayer(CollisionLayer::Convex, CollisionComponent::CollisionResponse::None);
    capsuleComponent->SetModelHeight(height * 0.5f);
    capsuleComponent->SetIsVisibleDebugBox(false);
    //char debugBuffer[128];
    //sprintf_s(debugBuffer, sizeof(debugBuffer),
    //    "CapsuleComponent Layer: 0x%X, Mask: 0x%X\n",
    //    capsuleComponent->GetCollisionLayer(), capsuleComponent->GetCollisionMask());
    //OutputDebugStringA(debugBuffer);
    capsuleComponent->Initialize();

    // ビームチャージ音コンポーネントを追加
    beamChargeAudioComponent = this->NewSceneComponent<AudioSourceComponent>("beamChargeAudioComponent", "skeltalComponent");
    beamChargeAudioComponent->SetSource(L"./Data/Sound/SE/beam_charge.wav");
    beamChargeAudioComponent->SetLoopOption(1.48f, 0.313f);
    // ビーム発射音コンポーネントを追加
    beamLaunchAudioComponent = this->NewSceneComponent<AudioSourceComponent>("beamLaunchAudioComponent", "skeltalComponent");
    beamLaunchAudioComponent->SetSource(L"./Data/Sound/SE/beam_launch.wav");
    // エフェクトコンポーネントを追加
    effectChargeComponent = this->NewSceneComponent<class EffectComponent>("effectChargeComponet", "skeltalComponent");
    // アイテム取得音
    itemAudioComponent = this->NewSceneComponent<AudioSourceComponent>("itemAudioComponent", "skeltalComponent");
    itemAudioComponent->SetSource(L"./Data/Sound/SE/energy.wav");
    AddHitCallback([&](std::pair<CollisionComponent*, CollisionComponent*> hitPair)
        {
            if (auto item = std::dynamic_pointer_cast<Stage>(hitPair.second->GetActor()))
            {
                return;
            }
            //std::string a = hitPair.second->GetActor()->GetName() + "is hit player";
            //OutputDebugStringA(a.c_str());
            if (auto item = std::dynamic_pointer_cast<PickUpItem>(hitPair.second->GetActor()))
            {// アイテムと当たったら
                //if (item.get() == lastHitPickUpItem)
                //{// 二度ヒットを防ぐため
                //    return;
                //}

                if (item->HasAlreadyHit(this))
                {
                    return;
                }

                if (!item->GetIsValid())
                {
                    return;
                }
                DirectX::XMFLOAT3 itemPos = item->GetPosition();
                DirectX::XMVECTOR ItemPosVec = DirectX::XMLoadFloat3(&itemPos);
                float leftDisSq = 0.0f;
                float rightDisSq = 0.0f;
                if (hitPair.first->name() == "boxHitLeftComponent" || hitPair.first->name() == "boxHitRightComponent")
                {
                    DirectX::XMFLOAT3 leftPos = leftComponent->GetComponentLocation();  // box は基準点がだんだんと変わり不安定だから
                    DirectX::XMVECTOR LeftPosVec = DirectX::XMLoadFloat3(&leftPos);
                    leftDisSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(ItemPosVec, LeftPosVec)));
                    //DirectX::XMFLOAT3 rightPos = hitPair.first->GetComponentLocation();
                    DirectX::XMFLOAT3 rightPos = rightComponent->GetComponentLocation();  // box は基準点がだんだんと変わり不安定だから
                    DirectX::XMVECTOR RightPosVec = DirectX::XMLoadFloat3(&rightPos);
                    rightDisSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(ItemPosVec, RightPosVec)));
                }
                if (leftDisSq >= rightDisSq)
                {// 右につく
                    if (rightItemMax <= rightItemCount)
                    {// アイテムが
                        return;
                    }
                    // ステージ上に湧かせるアイテムの max の値を決める
                    if (auto itemManager = GameManager::GetItemManager())
                    {
                        const auto& pos = item->GetPosition();
                        if (item->GetType() == PickUpItem::Type::RandomSpawn)
                        {// ランダムスポーンのアイテムを拾ったら　エリアに通知する
                            itemManager->DecreaseAreaItemCount(pos);
                        }
                    }
                    rightItemCount++;
                    //Transform
                    // プレイヤーの右側の描画スケールを大きくする
                    DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
                    rightScale.x += rightItemCount * scaleBigSize;
                    rightScale.y += rightItemCount * scaleBigSize;
                    rightScale.z += rightItemCount * scaleBigSize;
                    rightComponent->SetRelativeScaleDirect(rightScale);

                    // プレイヤーの右のアイテムの収集の当たり判定を大きくする
                    //float rightBoxWidth = rightScale.x * firstPlayerSideSize + radius;
                    float rightBoxWidth = rightFirstPos.x + rightScale.x * firstPlayerSideSize;
                    //boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, (rightBoxWidth) * 0.5f, (rightBoxWidth) * 0.5f);
                    boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    boxRightHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightBoxWidth) * 0.5f, firstRightBoxPosition.y, firstRightBoxPosition.z));

                    // プレイヤーの右のダメージ当たり判定を大きくする
                    float rightDamageRadius = (rightScale.x * firstPlayerSideSize) * 0.5f; // player の半径は足さない
                    playerDamageRight->ResizeSphere(rightDamageRadius);
                    //playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightDamageRadius + radius), rightDamageRadius, 0.0f));
                    playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3(rightFirstPos.x + rightDamageRadius, rightFirstPos.y + rightDamageRadius, rightFirstPos.z));

                    //item->SetValid(false);
                    item->SetPendingDestroy();

                    char debugBuffer[128];
                    sprintf_s(debugBuffer, sizeof(debugBuffer),
                        "rightItemCount%d\n",
                        rightItemCount);
                    OutputDebugStringA(debugBuffer);

                    //OutputDebugStringA("rightHit");
                }
                else
                {// 左につく
                    if (leftItemMax <= leftItemCount)
                    {// アイテムが
                        return;
                    }
                    // ステージ上に湧かせるアイテムの max の値を決める
                    if (auto itemManager = GameManager::GetItemManager())
                    {
                        if (item->GetType() == PickUpItem::Type::RandomSpawn)
                        {// ランダムスポーンのアイテムを拾ったら　エリアに通知する
                            const auto& pos = item->GetPosition();
                            itemManager->DecreaseAreaItemCount(pos);
                        }
                    }
                    leftItemCount++;
                    //Transform
                    // プレイヤーの左側の描画スケールを大きくする
                    DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
                    leftScale.x += leftItemCount * scaleBigSize;
                    leftScale.y += leftItemCount * scaleBigSize;
                    leftScale.z += leftItemCount * scaleBigSize;
                    leftComponent->SetRelativeScaleDirect(leftScale);
                    // プレイヤーの左のアイテムの収集の当たり判定を大きくする
                    float leftBoxWidth = -leftFirstPos.x + leftScale.x * firstPlayerSideSize;
                    boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, firstLeftBoxPosition.y, firstLeftBoxPosition.z));

                    //boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, (leftBoxWidth) * 0.5f, (leftBoxWidth) * 0.5f);
                    //boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    //boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, 0.0f, 0.0f));
                    // プレイヤーの左のダメージ当たり判定を大きくする
                    //float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player の半径は足さない
                    //playerDamageLeft->ResizeSphere(leftDamageRadius);
                    //playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftDamageRadius + radius), leftDamageRadius, 0.0f));
                    float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player の半径は足さない
                    playerDamageLeft->ResizeSphere(leftDamageRadius);
                    //playerDamageRight->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightDamageRadius + radius), rightDamageRadius, 0.0f));
                    playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(rightFirstPos.x + leftDamageRadius), rightFirstPos.y + leftDamageRadius, rightFirstPos.z));

                    //item->SetValid(false);
                    item->SetPendingDestroy();
                    char debugBuffer[128];
                    sprintf_s(debugBuffer, sizeof(debugBuffer),
                        "leftItemCount%d\n",
                        leftItemCount);
                    OutputDebugStringA(debugBuffer);

                    //OutputDebugStringA("leftHit");
                }
                // 取ったアイテムを記録しておく
                lastHitPickUpItem = item.get();
                // アイテムを取得したら
                itemAudioComponent->Play();
                item->RegisterHit(this);
                TutorialSystem::AchievedAction(TutorialStep::Collect);
                TutorialSystem::AchievedAction(TutorialStep::ManyCollect);
                TutorialSystem::AchievedAction(TutorialStep::ManyCollect2);
            }

            if (auto boss = std::dynamic_pointer_cast<RiderEnemy>(hitPair.second->GetActor()))
            {// ボスと当たったら
                if (hitPair.second->name() == "capsuleComponent")
                {
                    if (auto node = boss->activeNode)
                    {
                        // 無敵時間中かどうか
                        if (IsBossInvincible())
                        {
                            return;
                        }
                        int damage = 5; // ボスのダメージゲット
                        //if (node->GetName() == "Dash"||node->GetName()==
                        if(boss->GetEnableHit())
                        {// ボスが回転の突進をしている時　
#if 0
                            if (hitPair.first->name() == "playerDamageLeft")
                            {// 左に当たった時
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "playerDamageRight")
                            {// 右に当たった時
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "capsuleComponent")
                            {// 本体にのみ当たった時
                                if (!hitLeftThisFrame && !hitRightThisFrame)
                                {
                                    ApplyDirectHpDamage(damage);
                                }
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else
                            {// そのほかに当たった時
                                return;
                            }
#else
                            if (hitPair.first->name() == "boxHitLeftComponent")
                            {// 左に当たった時
                                hitLeftThisFrame = true;
                                std::string a = "boss damage  "+hitPair.first->name() + "is Hit\n";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "boxHitRightComponent")
                            {// 右に当たった時
                                hitRightThisFrame = true;
                                std::string a = "boss damage "+hitPair.first->name() + "is Hit\n";
                                OutputDebugStringA(a.c_str());
                            }
                            //else if (hitPair.second->name() == "capsuleComponent")
                            //{// 本体にのみ当たった時
                            //    if (!hitLeftThisFrame && !hitRightThisFrame)
                            //    {
                            //        ApplyDirectHpDamage(damage);
                            //    }
                            //    std::string a = hitPair.second->name() + "is Hit";
                            //    OutputDebugStringA(a.c_str());
                            //}
                            else
                            {// そのほかに当たった時
                                return;
                            }
#endif // 0
                            // 初回だけダメージを登録する
                            if (!hasDamageThisFrame)
                            {// ダメージ設定をしていなかったら
                                hasDamageThisFrame = true;
                                currentFrameDamage = damage;
                                applyInvincibilityNextFrame = true;

                                // 無敵時間間隔設定
                                SetBossInvincible();
                            }
                        }
                    }
                }
                else if (hitPair.second->name() == "bossHand")
                {// ボスの手に当たったら
                    if (auto node = boss->activeNode)
                    {
                        // 無敵時間中かどうか
                        if (IsBossInvincible())
                        {
                            return;
                        }
                        int damage = 4; // ボスのダメージゲット
                        if (node->GetName() == "Normal")
                        {
#if 0
                            if (hitPair.first->name() == "playerDamageLeft")
                            {// 左に当たった時
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "playerDamageRight")
                            {// 右に当たった時
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "capsuleComponent")
                            {// 本体にのみ当たった時
                                if (!hitLeftThisFrame && !hitRightThisFrame)
                                {
                                    ApplyDirectHpDamage(damage);
                                }
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else
                            {// そのほかに当たった時
                                return;
                            }
#else

                            if (hitPair.first->name() == "boxHitLeftComponent")
                            {// 左に当たった時
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "boxHitRightComponent")
                            {// 右に当たった時
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            //else if (hitPair.second->name() == "capsuleComponent")
                            //{// 本体にのみ当たった時
                            //    if (!hitLeftThisFrame && !hitRightThisFrame)
                            //    {
                            //        ApplyDirectHpDamage(damage);
                            //    }
                            //    std::string a = hitPair.second->name() + "is Hit";
                            //    OutputDebugStringA(a.c_str());
                            //}
                            else
                            {// そのほかに当たった時
                                return;
                            }
#endif // 0
                            // 初回だけダメージを登録する
                            if (!hasDamageThisFrame)
                            {// ダメージ設定をしていなかったら
                                hasDamageThisFrame = true;
                                currentFrameDamage = damage;
                                applyInvincibilityNextFrame = true;

                                // 無敵時間間隔設定
                                SetBossInvincible();
                            }
                        }
                    }
                }
            }
        });

    // 入力用のコンポーネントを追加
    inputComponent = this->NewSceneComponent<class InputComponent>("inputComponent", "skeltalComponent");
    //inputComponent->BindAction("Jump", [&]()
    //    {
    //        ChangeState(std::make_shared<JumpStartState>());
    //        dynamic_cast<SkeltalMeshComponent*>(GetComponentByName("skeltalComponent").get())->SetAnimationClip(3);
    //    });
    //inputComponent->BindActionAndButton(GamePad::Button::y, "Jump", TriggerMode::none); //[v]

    // 移動用コンポーネントを追加
    movementComponent = this->NewSceneComponent<class MovementComponent>("movementComponent", "skeltalComponent");

    // 回転用コンポーネントを追加
    rotationComponent = this->NewSceneComponent<class RotationComponent>("rotationComponet", "skeltalComponent");

    OutputDebugStringA(("Actor::Initialize called. rootComponent_ use_count = " + std::to_string(rootComponent_.use_count()) + "\n").c_str());
}



void Player::Update(float elapsedTime)
{
    // これは絶対入れる　アニメーションの更新をしているから
    Character::Update(elapsedTime);

    if (GameManager::GetGameTimerStart() && !onceFrag)
    {// ゲームが開始されたら
        state = State::Idle;
        onceFrag = true;
    }

    ItemColor(elapsedTime);

    // ステージ境界
    DirectX::XMFLOAT3 pos = GetPosition();
    pos.x = std::clamp(pos.x, -21.0f, 21.0f);
    pos.z = std::clamp(pos.z, -16.0f, 16.0f);
    pos.y = 0.8f;
    SetPosition(pos);

    float leftT = std::clamp(static_cast<float>(leftItemCount) / static_cast<float>(leftItemMax), 0.0f, 1.0f);
    float curveT = 1.0f - (1.0f - leftT) * (1.0f - leftT);
    leftComponent->model->emission = std::lerp(0.0f, 5.0f, curveT);
    float rightT = std::clamp(static_cast<float>(rightItemCount) / static_cast<float>(rightItemMax), 0.0f, 1.0f);
    float curveRT = 1.0f - (1.0f - rightT) * (1.0f - rightT);
    rightComponent->model->emission = std::lerp(0.0f, 5.0f, curveRT);
    // 歯車境界
    //if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(ActorManager::GetActorByName("enemy")))
    //{
    //    if (enemy->IsEnemyJumping())
    //    {
    //        DirectX::XMFLOAT3 gearPos = enemy->GetJumpPosition();
    //        // ギアの半径
    //        float radius = 2.0f;
    //        
    //        // 現在位置とギア中心のXZ差分
    //        float dx = pos.x - gearPos.x;
    //        float dz = pos.z - gearPos.z;

    //        float distSq = dx * dx + dz * dz;
    //        float radiusSq = radius * radius;

    //        if (distSq < radiusSq)
    //        {
    //            float dist = std::sqrt(distSq);
    //            float pushDist = radius - dist;

    //            float nx = dx / dist;
    //            float nz = dz / dist;

    //            pos.x += nx * pushDist;
    //            pos.z += nz * pushDist;

    //            SetPosition(pos);
    //        }
    //    }
    //};


    DirectX::XMFLOAT3 leftPos = boxLeftHitComponent->GetRelativeLocation();
    leftPos.y = leftFirstPos.y;
    boxLeftHitComponent->SetRelativeLocationDirect(leftPos);

    DirectX::XMFLOAT3 rightPos = boxRightHitComponent->GetRelativeLocation();
    rightPos.y = rightFirstPos.y;
    boxRightHitComponent->SetRelativeLocationDirect(rightPos);


    {// UI
        if (GameObject* hpGuage = ObjectManager::Find("HPGuage"))
        {
            float normalizeHp = static_cast<float>(hp / static_cast<float>(maxHp));
            hpGuage->GetComponent<Mask>()->valueX = normalizeHp;

            float normalizedLeftItem = static_cast<float>(leftItemCount / static_cast<float>(leftItemMax));
            ObjectManager::Find("LeftGauge")->GetComponent<Mask>()->valueY = normalizedLeftItem;
            float normalizedRightItem = static_cast<float>(rightItemCount / static_cast<float>(rightItemMax));
            ObjectManager::Find("RightGauge")->GetComponent<Mask>()->valueY = normalizedRightItem;
        }
    }
    {// 無敵時間の更新
        if (invisibleTime > 0.0f)
        {
            invisibleTime -= elapsedTime;
        }
        if (bossInvisibleTime > 0.0f)
        {
            bossInvisibleTime -= elapsedTime;
        }
    }

    // 左右の描画の位置を取るため
    //rightFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("R_core_FK", skeltalMeshComponent->model->nodes);
    //leftFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("L_core_FK", skeltalMeshComponent->model->nodes);

    {// EraseInArea に当たった時のダメージ処理
        int damageLeft = 0;
        int damageRight = 0;

        if (hitLeftThisFrame && hitRightThisFrame)
        {// 両方に当たった
            int half = currentFrameDamage / 2;
            damageLeft = half;
            damageRight = half;
        }
        else if (hitLeftThisFrame && !hitRightThisFrame)
        {// 左にだけヒットした場合
            damageLeft = currentFrameDamage;
        }
        else if (hitRightThisFrame && !hitLeftThisFrame)
        {// 右にだけヒットした場合
            damageRight = currentFrameDamage;
        }

        ApplyDamageToLeft(damageLeft);
        ApplyDamageToRight(damageRight);

        if (applyInvincibilityNextFrame)
        {
            SetInvincible();  // この時点で無敵時間開始
            applyInvincibilityNextFrame = false;
        }
        // playerSide の scale を変更する
        DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
        leftScale.x += leftItemCount * scaleBigSize;
        leftScale.y += leftItemCount * scaleBigSize;
        leftScale.z += leftItemCount * scaleBigSize;
        leftComponent->SetRelativeScaleDirect(leftScale);
        DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
        rightScale.x += rightItemCount * scaleBigSize;
        rightScale.y += rightItemCount * scaleBigSize;
        rightScale.z += rightItemCount * scaleBigSize;
        rightComponent->SetRelativeScaleDirect(rightScale);

        UpdateItemVisualShrink(elapsedTime);
        ResetHitFlags();
    }
    {// プレイヤーの回転スピードなど調整
        float currentItemCount = static_cast<float>(leftItemCount + rightItemCount);
        float maxItemCount = static_cast<float>(leftItemMax + rightItemMax);

        float t = std::clamp(currentItemCount / maxItemCount, 0.0f, 1.0f);
        currentSpeed = std::lerp(maxSpeed, minSpeed, t);
        // スピード調整
        movementComponent->SetSpeed(currentSpeed);
        // 回転調整
        Turn(elapsedTime);
        if (leftItemCount + rightItemCount == 0)
        {// アイテムを持っていないとき
            currentSpeed = noItemSpeed;
        }


        {// チュートリアルの移動判定
            DirectX::XMFLOAT3 dir = inputComponent->GetMoveInput();
            if (!(std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON))
            {// 入力値が0.0fよりも大きい場合
                TutorialSystem::AchievedAction(TutorialStep::Move);
            }
        }

        if (TutorialSystem::GetCurrentStep() == TutorialStep::ManyCollect || TutorialSystem::GetCurrentStep() == TutorialStep::ManyCollect2)
        {
            state = State::CantChargeBeam;
        }


        // player の回転の処理
        //{
        //    DirectX::XMFLOAT3 moveDir = inputComponent->GetMoveInput();
        //    float lenSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y + moveDir.z * moveDir.z;
        //    if (lenSq > 0.0001f)
        //    {
        //        rotationComponent->SetDirection(moveDir);
        //    }
        //}
    }

    switch (state)
    {
    case Player::State::Idle:
        currentTurnSpeed = maxTurnSpeed;
        TryStartCharge();
        break;
    case Player::State::Running:
        currentTurnSpeed = maxTurnSpeed;
        TryStartCharge();
        break;
    case Player::State::StartCharge:
        currentTurnSpeed = minTurnSpeed;
        //if (effectChargeComponent->GetEffectState() == EffectComponent::EffectState::Ending)
        if (InputSystem::GetInputState("MouseLeft", InputStateMask::Release))
        {// チャージが終わったら
            state = Player::State::FireBeam;
        }
        break;
    case Player::State::FireBeam:
        TutorialSystem::AchievedAction(TutorialStep::SecondAttack);
        TutorialSystem::AchievedAction(TutorialStep::FirstAttack);

        FireBeam();
        state = Player::State::Idle;
        break;
    case Player::State::FinishBeam:
        break;
    case Player::State::Attack:
        break;
    case Player::State::CantChargeBeam:
        currentTurnSpeed = maxTurnSpeed;
        break;
    case Player::State::CantMoveCharge:
        currentTurnSpeed = 0.0f;
        currentSpeed = 0.0f;
    default:
        break;
    }


    //float itemCount = static_cast<float>(hasRightItems.size() + hasLeftItems.size());
#if 0
    float itemCount = static_cast<float>(rightItemCount + leftItemCount);


    if (InputSystem::GetInputState("Enter", InputStateMask::Trigger) && itemCount > 0)
    {
        DirectX::XMFLOAT3 dir = GetForward();
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.x += dir.x * 1.0f;
        pos.z += dir.z * 1.0f;
        pos.y += 0.5f;

        //// エフェクトコンポーネントに伝達
        effectChargeComponent->SetWorldLocationDirect(pos);
        effectChargeComponent->SetEffectType(EffectComponent::EffectType::BeamCharge);
        effectChargeComponent->SetEffectPower(itemCount);
        effectChargeComponent->SetEffectDuration(1.5f);
        effectChargeComponent->Activate();
    }

    if (effectChargeComponent->GetEffectState() == EffectComponent::EffectState::Ending)
    {// 溜めが終わったら、  Beam を生成する
        DirectX::XMFLOAT3 dir = GetForward();
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.x += dir.x * 1.0f;
        pos.z += dir.z * 1.0f;
        pos.y += 0.5f;

        // Beam を生成する
        auto beam = ActorManager::CreateAndRegisterActor<Beam>("beam", false);
        beam->SetItemPower(itemCount);
        beam->SetItemCount(rightItemCount + leftItemCount);
        float itemPower = itemCount * 10.0f;
        float speed = 10.0f;
        beam->SetTempPosition(pos);
        beam->SetTempMass(itemPower);
        beam->Initialize();
        beam->PostInitialize();
        auto sphere = std::dynamic_pointer_cast<SphereComponent>(beam->GetSceneComponentByName("sphereComponent"));
        sphere->SetKinematic(false);
        sphere->SetGravity(false);
        //sphere->SetMass(itemPower);
        //sphere->SetIntialVelocity(DirectX::XMFLOAT3(dir.x * itemPower, dir.y * itemPower, dir.z * itemPower));
        sphere->SetIntialVelocity(DirectX::XMFLOAT3(dir.x * speed, dir.y * speed, dir.z * speed));
        HasItemReset();
        effectChargeComponent->Deactivate();
    }

#endif // 0

    DirectX::XMFLOAT3 origin = GetPosition();
    //origin.z += -1.0f;
    origin.y += height * 0.75f;
    DirectX::XMFLOAT3 direction = GetForward();
    //DirectX::XMFLOAT3 direction = DirectX::XMFLOAT3(0.0f,0.0f,-1.0f);

    float distance = 3.0f;
    RaycastHit result;

    // プレイヤーの被弾時に色を変える処理
    if (isHitBlinking)
    {
        hitBlinkElapsed += elapsedTime;
        isRed = true;
        constexpr float blinkInterval = 0.1f; // 点滅間隔（秒）
        int blinkCount = static_cast<int>(hitBlinkElapsed / blinkInterval);
        isRed = (blinkCount % 2 == 0);

        //// 終了
        if (hitBlinkElapsed >= hitBlinkTotalTime)
        {
            isHitBlinking = false;
            isRed = false;
        }
    }
    if (isRed)
    {
        color.x = 3.0f;
        color.y = 0.0f;
        color.z = 0.0f;
    }
    else
    {
        color.x = 1.0f;
        color.y = 1.0f;
        color.z = 1.0f;
    }
    skeltalMeshComponent->model->cpuColor.x = color.x;
    skeltalMeshComponent->model->cpuColor.y = color.y;
    skeltalMeshComponent->model->cpuColor.z = color.z;

    //MyQueryCallback callback;

    //physx::PxRaycastBuffer hit;
    //physx::PxQueryFilterData filterData;
    //filterData.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePREFILTER;
    //filterData.data.word0 = 1;

    //bool q = Physics::Instance().GetScene()->raycast(
    //    physx::PxVec3(0, 10, 0),
    //    physx::PxVec3(0, -1, 0),
    //    100.0f,
    //    hit,
    //    physx::PxHitFlag::eDEFAULT,
    //    filterData,
    //    &callback);
    //HitResult hit;
    //if (Physics::Instance().RayCast(origin, direction, distance, hit))
    //{
    //    OutputDebugStringA(("Player is hit " + std::to_string(hit.position.x) + "\n").c_str());
    //}
    //if (Physics::Instance().SphereCast(origin, direction, distance, 0.3f, hit))
    //{// これうまく動いた
    //    //OutputDebugStringA(("Player is hit " + std::to_string(hit.position.x) + "\n").c_str());
    //}

    //auto shape = GetComponent<CapsuleComponent>();
    ////if (!shape)
    //{// これうまく動いた
    //    if (PhysicsTest::Instance().SphereCast(origin, direction, distance, 0.3f, result, shape->GetCollisionLayer(), shape->GetCollisionMask()))
    //    {
    //        //OutputDebugStringA(("Player is hit " + (result.actor->GetName()) + "\n").c_str());
    //        auto p = result.actor->GetPosition();
    //        int a = 0;
    //    }
    //}


    //if (animationController_->IsPlayAnimation() && isIdleEnd)
    //{
    //    animationController_->RequestStopLoop();
    //}
    //if (!animationController_->IsPlayAnimation())
    //{
    //    PlayAnimation("Win", false);
    //}

    //if (InputSystem::GetInputState("E"))
    //{
    //    //this->PlayAnimation("Run", true, true);
    //    isIdleEnd = true;
    //}
    //if (InputSystem::GetInputState("E"))
    //{
    //    this->PlayAnimation("Attack", false, true);

    //}
    //if (InputSystem::GetInputState("T"))
    //{
    //    this->PlayAnimation("Idle");
    //}
#if USE_IMGUI
    ImGui::Begin("Player");
    ImGui::Text("State: %s", [&]() {
        switch (state)
        {
        case Player::State::Idle: return "Idle";
        case Player::State::Running: return "Running";
        case Player::State::StartCharge: return "StartCharge";
        case Player::State::FireBeam: return "FireBeam";
        case Player::State::FinishBeam: return "FinishBeam";
        case Player::State::Attack: return "Attack";
        case Player::State::CantChargeBeam: return "CantChargeBeam";
        default: return "Unknown";
        }
        }());
    ImGui::ColorEdit3("playerDamage", &color.x);
    ImGui::DragFloat3("playerDamageColor", &color.x);
    ImGui::End();
#endif
}
// ビームをチャージする関数
void Player::TryStartCharge()
{
    float itemCount = static_cast<float>(rightItemCount + leftItemCount);
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger) && itemCount > 0)
    {
        // チャージの音を再生する
        beamChargeAudioComponent->Play(XAUDIO2_LOOP_INFINITE);

        DirectX::XMFLOAT3 pos = skeltalMeshComponent->GetJointWorldPosition("beam_FK");

        // エフェクトコンポーネントに伝達
        effectChargeComponent->SetWorldLocationDirect(pos);
        effectChargeComponent->SetEffectType(EffectComponent::EffectType::BeamCharge);
        effectChargeComponent->SetEffectPower(itemCount);
        DirectX::XMFLOAT3 dir = GetForward();
        effectChargeComponent->SetEffectForward(dir);
        //effectChargeComponent->SetEffectDuration(1.5f);
        effectChargeComponent->Activate();
        state = State::StartCharge;
    }
}

// ビームを発射する関数
void Player::FireBeam()
{
    // チャージの音を停止する
    beamChargeAudioComponent->Stop();
    // 発射音を再生する
    beamLaunchAudioComponent->Play();

    float beamItemPower = static_cast<float>(rightItemCount + leftItemCount);
    int itemCount = rightItemCount + leftItemCount;

    char debugBuffer[128];
    sprintf_s(debugBuffer, sizeof(debugBuffer),
        "beamItemCount%d\n",
        itemCount);
    OutputDebugStringA(debugBuffer);

    //if (effectChargeComponent->GetEffectState() == EffectComponent::EffectState::Ending)
    {// 溜めが終わったら、  Beam を生成する
        DirectX::XMFLOAT3 dir = GetForward();
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.x += dir.x * 1.0f;
        pos.z += dir.z * 1.0f;
        pos.y += 0.5f;
        float speed = 10.0f;
        DirectX::XMFLOAT3 vel = { dir.x * speed, dir.y * speed, dir.z * speed };
        char buf[256];
        sprintf_s(buf, sizeof(buf),
            "FireBeam: forward=(%.3f, %.3f, %.3f), pos=(%.3f, %.3f, %.3f), velocity=(%.3f, %.3f, %.3f)\n",
            dir.x, dir.y, dir.z,
            pos.x, pos.y, pos.z,
            vel.x, vel.y, vel.z);
        OutputDebugStringA(buf);
        // Beam を生成する
        auto beam = GetOwnerScene()->GetActorManager()->CreateAndRegisterActor<Beam>("beam", false);
        beam->SetItemPower(beamItemPower);
        float maxPower = static_cast<float>(rightItemMax + leftItemMax);
        beam->SetItemMaxPower(maxPower);
        beam->SetItemCount(rightItemCount + leftItemCount);
        float itemPower = beamItemPower * 10.0f;
        beam->SetTempPosition(pos);
        beam->SetTempMass(itemPower);
        beam->Initialize();
        beam->PostInitialize();
        beam->SetDirection(dir);
        //auto sphere = std::dynamic_pointer_cast<SphereComponent>(beam->GetSceneComponentByName("sphereComponent"));
        //sphere->SetKinematic(false);
        //sphere->SetGravity(false);
        //sphere->SetMass(itemPower);
        //sphere->SetIntialVelocity(DirectX::XMFLOAT3(dir.x * itemPower, dir.y * itemPower, dir.z * itemPower));
        //sphere->SetIntialVelocity(DirectX::XMFLOAT3(dir.x * speed, dir.y * speed, dir.z * speed));
        HasItemReset();
        effectChargeComponent->Deactivate();
    }
}


void Player::DrawImGuiDetails()
{
#ifdef USE_IMGUI

    //if (ImGui::TreeNode("Player"))
    {
        PhysicsTest::Instance().DebugShapePosition();
        // 現在のプレイヤーステートを表示
        //if (!stateStack.empty())
        //{
        //    std::vector<std::shared_ptr<PlayerState>> tempStack;
        //    std::stack<std::shared_ptr<PlayerState>> copyStack = stateStack; // コピーを作成

        //    // スタックをベクターにコピー
        //    while (!copyStack.empty())
        //    {
        //        tempStack.push_back(copyStack.top());
        //        copyStack.pop();
        //    }
        //    // スタックの順番を逆にして表示（Topが最上位になるように）
        //    ImGui::Text("Player States:");
        //    for (auto it = tempStack.rbegin(); it != tempStack.rend(); ++it)
        //    {
        //        ImGui::Text("- %s", (*it)->GetStateName().c_str()); // ステート名を表示
        //    }
        //}
        //else
        //{
        //    ImGui::Text("No Active State");
        //}
        //DirectX::XMFLOAT3 position = GetPosition();
        //ImGui::DragFloat3("Position", &position.x, 0.5f);
        //入力情報を取得
        //float ax = pad.ThumbStateLx();
        //float ay = pad.ThumbStateLy();
        ImGui::DragFloat("scaleBigSize", &scaleBigSize, 0.01f);
        ImGui::DragInt("Hp", &hp);
        ImGui::DragFloat("currentSpeed", &currentSpeed);
        ImGui::DragFloat("currentTurnSpeed", &currentTurnSpeed);
        //ImGui::DragFloat("ThumbStateLx", &ax, 0.5f);
        //ImGui::DragFloat("ThumbStateLy", &ay, 0.5f);
        //ImGui::DragFloat3("Velocity", &velocity.x, 0.5f);
        //ImGui::DragFloat3("intersectStagePosition", &intersectStagePosition.x, 0.5f);

        //ImGui::TreePop();
    }
#endif

}

// 遅延更新処理
void Player::LateUpdate(float elapsedTime)
{

}

void Player::Turn(float elapsedTime)
{
    using namespace DirectX;
#if 0
    //pad.Acquire();

//入力情報を取得
    float thumbStateLx = pad.ThumbStateLx();
    float thumbStateLy = pad.ThumbStateLy();


    DirectX::XMVECTOR PAD = XMVectorSet(thumbStateLx, 0.0f, thumbStateLy, 0.0f);
    PAD = DirectX::XMVector3Normalize(PAD);
    // 入力がほぼゼロなら回転しない
    if (std::abs(thumbStateLx) < 0.1f && std::abs(thumbStateLy) < 0.1f)
    {
        return;
    }

    //プレイヤーが向いている前方向ベクトルを求める
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //プレイヤーの３軸を求める
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////カメラの注視点と位置を取得する
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////カメラの座標軸を求める
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    //const float inputDeadZone = 0.0001f;

    ////スティックの入力をカメラの座標軸に変換して進行方向のベクトルを求める
    ////カメラ基準の移動方向
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    ////進行方向ベクトルとplayerの今の向いている方向の X 軸右ベクトルで内積する
    //float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(CameraBasedMoveDirection, PX));

    //if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    //{
    //    turningSpeed = maxTurningSpeed * turningSpeedFactor;
    //}
    // 


    //float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(CameraBasedMoveDirection, PZ));
    //if (cross > 0.0f)
    //{//左に回転する
    //    rotation.y -= rot;
    //}
    //else
    //{//右に回転する
    //    rotation.y += rot;
    //}

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //クォータニオンから回転行列を作成
    XMVECTOR Y = R.r[1];//モデルのローカルなY軸を取り出す
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, +DirectX::XMConvertToRadians(turningSpeed) * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
#else
    //rotation.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
#endif

#endif // 0
    float vx = inputComponent->GetThumbStateRx();
    float vz = inputComponent->GetThumbStateRy();
    float speed = DirectX::XMConvertToRadians(currentTurnSpeed) * elapsedTime;

    // 進行ベクトルがゼロベクトルの場合は処理する必要なし
    float length = sqrtf(vx * vx + vz * vz);
    if (length <= 0.001f)
    {
        return;
    }
    // 進行ベクトルを単位ベクトル化
    vx /= length;
    vz /= length;

    // 自身の回転値から前方向を求める
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    // 回転角を求めるため、2つの単位ベクトルの内積を計算する
    float dot = frontX * vx + frontZ * vz;

    // 内積値は-1.0～1.0で表現されており、2つの単位ベクトルの角度が
    // 小さいほど1.0に近づくという性質を利用して回転速度を調整する
    float rot = 1.0f - dot;

    if (rot > speed)
    {
        rot = speed;
    }

    // 左右判定を行うために２つの単位ベクトルの外積を計算する
    float cross = (frontX * vz) - (frontZ * vx);

    // 2Dの外積値が生の場合か負の場合かによって左右判定が行える
    // 左右判定を行うことによって左右回転を選択する
    if (cross < 0.0f)
    {
        angle.y += rot;
    }
    else
    {
        angle.y -= rot;
    }

    DirectX::XMFLOAT4 quaternion;
    DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMStoreFloat4(&quaternion, q);

    SetQuaternionRotation(quaternion);



#if 0
    float thumbStateRx = inputComponent->GetThumbStateRx();
    float thumbStateRy = inputComponent->GetThumbStateRy();

    DirectX::XMVECTOR PAD = XMVectorSet(thumbStateRx, 0.0f, thumbStateRy, 0.0f);
    PAD = DirectX::XMVector3Normalize(PAD);
    // 入力がほぼゼロなら回転しない
    if (std::abs(thumbStateRx) < 0.1f && std::abs(thumbStateRy) < 0.1f)
    {
        return;
    }

    //プレイヤーが向いている前方向ベクトルを求める
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //プレイヤーの３軸を求める
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////カメラの注視点と位置を取得する
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////カメラの座標軸を求める
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    const float inputDeadZone = 0.0001f;

    ////スティックの入力をカメラの座標軸に変換して進行方向のベクトルを求める
    ////カメラ基準の移動方向
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    DirectX::XMFLOAT3 moveDir = inputComponent->GetMoveInput();
    DirectX::XMVECTOR MoveDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&moveDir));
    //float lenSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y + moveDir.z * moveDir.z;
    //if (lenSq > 0.0001f)
    //{
    //    rotationComponent->SetDirection(moveDir);
    //}

    ////進行方向ベクトルとplayerの今の向いている方向の X 軸右ベクトルで内積する
    float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(MoveDirection, PX));

    if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    {
        turningSpeed = maxTurningSpeed * turningSpeedFactor;
    }



    float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(MoveDirection, PZ));
    if (cross > 0.0f)
    {//左に回転する
        angle.y -= rot;
    }
    else
    {//右に回転する
        angle.y += rot;
    }

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //クォータニオンから回転行列を作成
    XMVECTOR Y = R.r[1];//モデルのローカルなY軸を取り出す
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, +DirectX::XMConvertToRadians(turningSpeed) * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
#else
    angle.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
#endif
    DirectX::XMFLOAT4 quaternion;
    DirectX::XMVECTOR Quat = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
    DirectX::XMStoreFloat4(&quaternion, Quat);
    SetQuaternionRotation(quaternion);

#endif // 0
}

void Player::Move(float elapsedTime)
{
    //pad.Acquire();

    DirectX::XMFLOAT3 pos = GetPosition();
    //進行方向のベクトル取得
    DirectX::XMFLOAT3 moveVec = GetMoveVec();
    velocity.x = moveVec.x;
    velocity.z = moveVec.z;
    float moveSpeed = 5.0f * elapsedTime;
    pos.x += moveVec.x * moveSpeed;
    pos.z += moveVec.z * moveSpeed;
    SetPosition(pos);
    //position.x += velocity.x * moveSpeed;
    //position.z += velocity.z * moveSpeed;
    HandleInput(pad);
}

void Player::HandleInput(GamePad& pad)
{
    if (!stateStack.empty())
    {
        //stateStack.top()->HandleInput(*this, pad);
    }
}

void Player::PushState(std::shared_ptr<PlayerState> state)
{
    if (!stateStack.empty())
    {
        stateStack.top()->Exit(*this);
    }
    stateStack.push(state);
    state->Enter(*this);
}

void Player::PopState()
{
    if (!stateStack.empty())
    {//なくなる時
        stateStack.top()->Exit(*this);
        stateStack.pop();
    }
    if (!stateStack.empty())
    {//残っているステート
        stateStack.top()->Enter(*this);
    }
}

void Player::ChangeState(std::shared_ptr<PlayerState> state)
{
    if (!stateStack.empty())
    {
        stateStack.top()->Exit(*this);
        stateStack.pop();
    }
    stateStack.push(state);
    state->Enter(*this);
}

//当たった時の処理
void Player::Hit()
{
    hp -= 1;
}

//スティックの入力値から移動ベクトルを取得
DirectX::XMFLOAT3 Player::GetMoveVec()
{
    //pad.Acquire();
#if 0
    //入力情報を取得
    float ax = pad.ThumbStateLx();
    float ay = pad.ThumbStateLy();

    //カメラ方向とスティックの入力値によって進行方向を計算する
    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

    //カメラ右方向ベクトル[X軸]をXZ平面での単位ベクトルに変換
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = std::sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //単位ベクトル化
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //カメラの前方向のベクトル[Z軸]をXZ単位ベクトルに変換
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = std::sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //単位ベクトル化
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //進行ベクトルを計算する
    DirectX::XMFLOAT3 vec;
    vec.x = (cameraRightX * ax) + (cameraFrontX * ay);
    vec.z = (cameraRightZ * ax) + (cameraFrontZ * ay);
    //Y軸方向には移動しない
    vec.y = 0.0f;

    return vec;
#endif
    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

