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

// UI�Œǉ�
#include "Widgets/ObjectManager.h"
#include "Widgets/GameObject.h"
#include "Widgets/Mask.h"

// �`���[�g���A���Ɏg�p
#include "Game/Managers/TutorialSystem.h"

void Player::Initialize(const Transform& transform)
{
    // �`��p�R���|�[�l���g��ǉ�
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
    // �A�j���[�V�����R���g���[���[���쐬
    auto controller = std::make_shared<AnimationController>(skeltalMeshComponent.get());
    controller->AddAnimation("Idle", 0);
    //controller->AddAnimation("Win", 1);
    // �A�j���[�V�����R���g���[���[��character�ɒǉ�
    this->SetAnimationController(controller);
    //PlayAnimation("Idle",true,false);

    //leftFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("R_core_FK", skeltalMeshComponent->model->nodes);
    //rightFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("L_core_FK", skeltalMeshComponent->model->nodes);


    // �v���C���[�̍��̌�����
    leftComponent = this->NewSceneComponent<class SkeltalMeshComponent>("leftComponent", "skeltalComponent");
    leftComponent->SetModel("./Data/Models/Characters/Player/PlayerSide/player_side_left.gltf");
    leftComponent->SetRelativeLocationDirect(leftFirstPos);
    //CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", leftComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPlayerSidePS.cso", leftComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    leftComponent->model->emission = 0.0f;
    leftComponent->model->cpuColor = { 1.0f,1.0f,1.0f };
    // �G�̍U���������鍶�� 
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

    // �����̃A�C�e�����W�p�����蔻��
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

    // �v���C���[�̉E�̌�����
    rightComponent = this->NewSceneComponent<class SkeltalMeshComponent>("rightComponent", "skeltalComponent");
    rightComponent->SetModel("./Data/Models/Characters/Player/PlayerSide/player_side_right.gltf");
    rightComponent->SetRelativeLocationDirect(rightFirstPos);
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPlayerSidePS.cso", rightComponent->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    rightComponent->model->emission = 0.0f;
    leftComponent->model->cpuColor = { 1.0f,1.0f,1.0f };
    // �G�̍U����������E�� 
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

    // �E���̃A�C�e�����W�p�����蔻��
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

    //// �G����̍U�����󂯂铖���蔻��p�̃R���|�[�l���g��ǉ�
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

    // �r�[���`���[�W���R���|�[�l���g��ǉ�
    beamChargeAudioComponent = this->NewSceneComponent<AudioSourceComponent>("beamChargeAudioComponent", "skeltalComponent");
    beamChargeAudioComponent->SetSource(L"./Data/Sound/SE/beam_charge.wav");
    beamChargeAudioComponent->SetLoopOption(1.48f, 0.313f);
    // �r�[�����ˉ��R���|�[�l���g��ǉ�
    beamLaunchAudioComponent = this->NewSceneComponent<AudioSourceComponent>("beamLaunchAudioComponent", "skeltalComponent");
    beamLaunchAudioComponent->SetSource(L"./Data/Sound/SE/beam_launch.wav");
    // �G�t�F�N�g�R���|�[�l���g��ǉ�
    effectChargeComponent = this->NewSceneComponent<class EffectComponent>("effectChargeComponet", "skeltalComponent");
    // �A�C�e���擾��
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
            {// �A�C�e���Ɠ���������
                //if (item.get() == lastHitPickUpItem)
                //{// ��x�q�b�g��h������
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
                    DirectX::XMFLOAT3 leftPos = leftComponent->GetComponentLocation();  // box �͊�_�����񂾂�ƕς��s���肾����
                    DirectX::XMVECTOR LeftPosVec = DirectX::XMLoadFloat3(&leftPos);
                    leftDisSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(ItemPosVec, LeftPosVec)));
                    //DirectX::XMFLOAT3 rightPos = hitPair.first->GetComponentLocation();
                    DirectX::XMFLOAT3 rightPos = rightComponent->GetComponentLocation();  // box �͊�_�����񂾂�ƕς��s���肾����
                    DirectX::XMVECTOR RightPosVec = DirectX::XMLoadFloat3(&rightPos);
                    rightDisSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(ItemPosVec, RightPosVec)));
                }
                if (leftDisSq >= rightDisSq)
                {// �E�ɂ�
                    if (rightItemMax <= rightItemCount)
                    {// �A�C�e����
                        return;
                    }
                    // �X�e�[�W��ɗN������A�C�e���� max �̒l�����߂�
                    if (auto itemManager = GameManager::GetItemManager())
                    {
                        const auto& pos = item->GetPosition();
                        if (item->GetType() == PickUpItem::Type::RandomSpawn)
                        {// �����_���X�|�[���̃A�C�e�����E������@�G���A�ɒʒm����
                            itemManager->DecreaseAreaItemCount(pos);
                        }
                    }
                    rightItemCount++;
                    //Transform
                    // �v���C���[�̉E���̕`��X�P�[����傫������
                    DirectX::XMFLOAT3 rightScale = { 1.0f,1.0f,1.0f };
                    rightScale.x += rightItemCount * scaleBigSize;
                    rightScale.y += rightItemCount * scaleBigSize;
                    rightScale.z += rightItemCount * scaleBigSize;
                    rightComponent->SetRelativeScaleDirect(rightScale);

                    // �v���C���[�̉E�̃A�C�e���̎��W�̓����蔻���傫������
                    //float rightBoxWidth = rightScale.x * firstPlayerSideSize + radius;
                    float rightBoxWidth = rightFirstPos.x + rightScale.x * firstPlayerSideSize;
                    //boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, (rightBoxWidth) * 0.5f, (rightBoxWidth) * 0.5f);
                    boxRightHitComponent->ResizeBox((rightBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    boxRightHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3((rightBoxWidth) * 0.5f, firstRightBoxPosition.y, firstRightBoxPosition.z));

                    // �v���C���[�̉E�̃_���[�W�����蔻���傫������
                    float rightDamageRadius = (rightScale.x * firstPlayerSideSize) * 0.5f; // player �̔��a�͑����Ȃ�
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
                {// ���ɂ�
                    if (leftItemMax <= leftItemCount)
                    {// �A�C�e����
                        return;
                    }
                    // �X�e�[�W��ɗN������A�C�e���� max �̒l�����߂�
                    if (auto itemManager = GameManager::GetItemManager())
                    {
                        if (item->GetType() == PickUpItem::Type::RandomSpawn)
                        {// �����_���X�|�[���̃A�C�e�����E������@�G���A�ɒʒm����
                            const auto& pos = item->GetPosition();
                            itemManager->DecreaseAreaItemCount(pos);
                        }
                    }
                    leftItemCount++;
                    //Transform
                    // �v���C���[�̍����̕`��X�P�[����傫������
                    DirectX::XMFLOAT3 leftScale = { 1.0f,1.0f,1.0f };
                    leftScale.x += leftItemCount * scaleBigSize;
                    leftScale.y += leftItemCount * scaleBigSize;
                    leftScale.z += leftItemCount * scaleBigSize;
                    leftComponent->SetRelativeScaleDirect(leftScale);
                    // �v���C���[�̍��̃A�C�e���̎��W�̓����蔻���傫������
                    float leftBoxWidth = -leftFirstPos.x + leftScale.x * firstPlayerSideSize;
                    boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, firstLeftBoxPosition.y, firstLeftBoxPosition.z));

                    //boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, (leftBoxWidth) * 0.5f, (leftBoxWidth) * 0.5f);
                    //boxLeftHitComponent->ResizeBox((leftBoxWidth) * 0.5f, firstHalfBoxExtent.y, firstHalfBoxExtent.z);
                    //boxLeftHitComponent->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftBoxWidth) * 0.5f, 0.0f, 0.0f));
                    // �v���C���[�̍��̃_���[�W�����蔻���傫������
                    //float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player �̔��a�͑����Ȃ�
                    //playerDamageLeft->ResizeSphere(leftDamageRadius);
                    //playerDamageLeft->SetRelativeLocationDirect(DirectX::XMFLOAT3(-(leftDamageRadius + radius), leftDamageRadius, 0.0f));
                    float leftDamageRadius = (leftScale.x * firstPlayerSideSize) * 0.5f; // player �̔��a�͑����Ȃ�
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
                // ������A�C�e�����L�^���Ă���
                lastHitPickUpItem = item.get();
                // �A�C�e�����擾������
                itemAudioComponent->Play();
                item->RegisterHit(this);
                TutorialSystem::AchievedAction(TutorialStep::Collect);
                TutorialSystem::AchievedAction(TutorialStep::ManyCollect);
                TutorialSystem::AchievedAction(TutorialStep::ManyCollect2);
            }

            if (auto boss = std::dynamic_pointer_cast<RiderEnemy>(hitPair.second->GetActor()))
            {// �{�X�Ɠ���������
                if (hitPair.second->name() == "capsuleComponent")
                {
                    if (auto node = boss->activeNode)
                    {
                        // ���G���Ԓ����ǂ���
                        if (IsBossInvincible())
                        {
                            return;
                        }
                        int damage = 5; // �{�X�̃_���[�W�Q�b�g
                        //if (node->GetName() == "Dash"||node->GetName()==
                        if(boss->GetEnableHit())
                        {// �{�X����]�̓ːi�����Ă��鎞�@
#if 0
                            if (hitPair.first->name() == "playerDamageLeft")
                            {// ���ɓ���������
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "playerDamageRight")
                            {// �E�ɓ���������
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "capsuleComponent")
                            {// �{�̂ɂ̂ݓ���������
                                if (!hitLeftThisFrame && !hitRightThisFrame)
                                {
                                    ApplyDirectHpDamage(damage);
                                }
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else
                            {// ���̂ق��ɓ���������
                                return;
                            }
#else
                            if (hitPair.first->name() == "boxHitLeftComponent")
                            {// ���ɓ���������
                                hitLeftThisFrame = true;
                                std::string a = "boss damage  "+hitPair.first->name() + "is Hit\n";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "boxHitRightComponent")
                            {// �E�ɓ���������
                                hitRightThisFrame = true;
                                std::string a = "boss damage "+hitPair.first->name() + "is Hit\n";
                                OutputDebugStringA(a.c_str());
                            }
                            //else if (hitPair.second->name() == "capsuleComponent")
                            //{// �{�̂ɂ̂ݓ���������
                            //    if (!hitLeftThisFrame && !hitRightThisFrame)
                            //    {
                            //        ApplyDirectHpDamage(damage);
                            //    }
                            //    std::string a = hitPair.second->name() + "is Hit";
                            //    OutputDebugStringA(a.c_str());
                            //}
                            else
                            {// ���̂ق��ɓ���������
                                return;
                            }
#endif // 0
                            // ���񂾂��_���[�W��o�^����
                            if (!hasDamageThisFrame)
                            {// �_���[�W�ݒ�����Ă��Ȃ�������
                                hasDamageThisFrame = true;
                                currentFrameDamage = damage;
                                applyInvincibilityNextFrame = true;

                                // ���G���ԊԊu�ݒ�
                                SetBossInvincible();
                            }
                        }
                    }
                }
                else if (hitPair.second->name() == "bossHand")
                {// �{�X�̎�ɓ���������
                    if (auto node = boss->activeNode)
                    {
                        // ���G���Ԓ����ǂ���
                        if (IsBossInvincible())
                        {
                            return;
                        }
                        int damage = 4; // �{�X�̃_���[�W�Q�b�g
                        if (node->GetName() == "Normal")
                        {
#if 0
                            if (hitPair.first->name() == "playerDamageLeft")
                            {// ���ɓ���������
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "playerDamageRight")
                            {// �E�ɓ���������
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "capsuleComponent")
                            {// �{�̂ɂ̂ݓ���������
                                if (!hitLeftThisFrame && !hitRightThisFrame)
                                {
                                    ApplyDirectHpDamage(damage);
                                }
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else
                            {// ���̂ق��ɓ���������
                                return;
                            }
#else

                            if (hitPair.first->name() == "boxHitLeftComponent")
                            {// ���ɓ���������
                                hitLeftThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            else if (hitPair.first->name() == "boxHitRightComponent")
                            {// �E�ɓ���������
                                hitRightThisFrame = true;
                                std::string a = hitPair.first->name() + "is Hit";
                                OutputDebugStringA(a.c_str());
                            }
                            //else if (hitPair.second->name() == "capsuleComponent")
                            //{// �{�̂ɂ̂ݓ���������
                            //    if (!hitLeftThisFrame && !hitRightThisFrame)
                            //    {
                            //        ApplyDirectHpDamage(damage);
                            //    }
                            //    std::string a = hitPair.second->name() + "is Hit";
                            //    OutputDebugStringA(a.c_str());
                            //}
                            else
                            {// ���̂ق��ɓ���������
                                return;
                            }
#endif // 0
                            // ���񂾂��_���[�W��o�^����
                            if (!hasDamageThisFrame)
                            {// �_���[�W�ݒ�����Ă��Ȃ�������
                                hasDamageThisFrame = true;
                                currentFrameDamage = damage;
                                applyInvincibilityNextFrame = true;

                                // ���G���ԊԊu�ݒ�
                                SetBossInvincible();
                            }
                        }
                    }
                }
            }
        });

    // ���͗p�̃R���|�[�l���g��ǉ�
    inputComponent = this->NewSceneComponent<class InputComponent>("inputComponent", "skeltalComponent");
    //inputComponent->BindAction("Jump", [&]()
    //    {
    //        ChangeState(std::make_shared<JumpStartState>());
    //        dynamic_cast<SkeltalMeshComponent*>(GetComponentByName("skeltalComponent").get())->SetAnimationClip(3);
    //    });
    //inputComponent->BindActionAndButton(GamePad::Button::y, "Jump", TriggerMode::none); //[v]

    // �ړ��p�R���|�[�l���g��ǉ�
    movementComponent = this->NewSceneComponent<class MovementComponent>("movementComponent", "skeltalComponent");

    // ��]�p�R���|�[�l���g��ǉ�
    rotationComponent = this->NewSceneComponent<class RotationComponent>("rotationComponet", "skeltalComponent");

    OutputDebugStringA(("Actor::Initialize called. rootComponent_ use_count = " + std::to_string(rootComponent_.use_count()) + "\n").c_str());
}



void Player::Update(float elapsedTime)
{
    // ����͐�Γ����@�A�j���[�V�����̍X�V�����Ă��邩��
    Character::Update(elapsedTime);

    if (GameManager::GetGameTimerStart() && !onceFrag)
    {// �Q�[�����J�n���ꂽ��
        state = State::Idle;
        onceFrag = true;
    }

    ItemColor(elapsedTime);

    // �X�e�[�W���E
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
    // ���ԋ��E
    //if (auto enemy = std::dynamic_pointer_cast<RiderEnemy>(ActorManager::GetActorByName("enemy")))
    //{
    //    if (enemy->IsEnemyJumping())
    //    {
    //        DirectX::XMFLOAT3 gearPos = enemy->GetJumpPosition();
    //        // �M�A�̔��a
    //        float radius = 2.0f;
    //        
    //        // ���݈ʒu�ƃM�A���S��XZ����
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
    {// ���G���Ԃ̍X�V
        if (invisibleTime > 0.0f)
        {
            invisibleTime -= elapsedTime;
        }
        if (bossInvisibleTime > 0.0f)
        {
            bossInvisibleTime -= elapsedTime;
        }
    }

    // ���E�̕`��̈ʒu����邽��
    //rightFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("R_core_FK", skeltalMeshComponent->model->nodes);
    //leftFirstPos = skeltalMeshComponent->model->GetJointLocalPosition("L_core_FK", skeltalMeshComponent->model->nodes);

    {// EraseInArea �ɓ����������̃_���[�W����
        int damageLeft = 0;
        int damageRight = 0;

        if (hitLeftThisFrame && hitRightThisFrame)
        {// �����ɓ�������
            int half = currentFrameDamage / 2;
            damageLeft = half;
            damageRight = half;
        }
        else if (hitLeftThisFrame && !hitRightThisFrame)
        {// ���ɂ����q�b�g�����ꍇ
            damageLeft = currentFrameDamage;
        }
        else if (hitRightThisFrame && !hitLeftThisFrame)
        {// �E�ɂ����q�b�g�����ꍇ
            damageRight = currentFrameDamage;
        }

        ApplyDamageToLeft(damageLeft);
        ApplyDamageToRight(damageRight);

        if (applyInvincibilityNextFrame)
        {
            SetInvincible();  // ���̎��_�Ŗ��G���ԊJ�n
            applyInvincibilityNextFrame = false;
        }
        // playerSide �� scale ��ύX����
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
    {// �v���C���[�̉�]�X�s�[�h�Ȃǒ���
        float currentItemCount = static_cast<float>(leftItemCount + rightItemCount);
        float maxItemCount = static_cast<float>(leftItemMax + rightItemMax);

        float t = std::clamp(currentItemCount / maxItemCount, 0.0f, 1.0f);
        currentSpeed = std::lerp(maxSpeed, minSpeed, t);
        // �X�s�[�h����
        movementComponent->SetSpeed(currentSpeed);
        // ��]����
        Turn(elapsedTime);
        if (leftItemCount + rightItemCount == 0)
        {// �A�C�e���������Ă��Ȃ��Ƃ�
            currentSpeed = noItemSpeed;
        }


        {// �`���[�g���A���̈ړ�����
            DirectX::XMFLOAT3 dir = inputComponent->GetMoveInput();
            if (!(std::abs(dir.x - 0.0f) <= FLT_EPSILON && std::abs(dir.y - 0.0f) <= FLT_EPSILON && std::abs(dir.z - 0.0f) <= FLT_EPSILON))
            {// ���͒l��0.0f�����傫���ꍇ
                TutorialSystem::AchievedAction(TutorialStep::Move);
            }
        }

        if (TutorialSystem::GetCurrentStep() == TutorialStep::ManyCollect || TutorialSystem::GetCurrentStep() == TutorialStep::ManyCollect2)
        {
            state = State::CantChargeBeam;
        }


        // player �̉�]�̏���
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
        {// �`���[�W���I�������
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

        //// �G�t�F�N�g�R���|�[�l���g�ɓ`�B
        effectChargeComponent->SetWorldLocationDirect(pos);
        effectChargeComponent->SetEffectType(EffectComponent::EffectType::BeamCharge);
        effectChargeComponent->SetEffectPower(itemCount);
        effectChargeComponent->SetEffectDuration(1.5f);
        effectChargeComponent->Activate();
    }

    if (effectChargeComponent->GetEffectState() == EffectComponent::EffectState::Ending)
    {// ���߂��I�������A  Beam �𐶐�����
        DirectX::XMFLOAT3 dir = GetForward();
        DirectX::XMFLOAT3 pos = GetPosition();
        pos.x += dir.x * 1.0f;
        pos.z += dir.z * 1.0f;
        pos.y += 0.5f;

        // Beam �𐶐�����
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

    // �v���C���[�̔�e���ɐF��ς��鏈��
    if (isHitBlinking)
    {
        hitBlinkElapsed += elapsedTime;
        isRed = true;
        constexpr float blinkInterval = 0.1f; // �_�ŊԊu�i�b�j
        int blinkCount = static_cast<int>(hitBlinkElapsed / blinkInterval);
        isRed = (blinkCount % 2 == 0);

        //// �I��
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
    //{// ���ꂤ�܂�������
    //    //OutputDebugStringA(("Player is hit " + std::to_string(hit.position.x) + "\n").c_str());
    //}

    //auto shape = GetComponent<CapsuleComponent>();
    ////if (!shape)
    //{// ���ꂤ�܂�������
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
// �r�[�����`���[�W����֐�
void Player::TryStartCharge()
{
    float itemCount = static_cast<float>(rightItemCount + leftItemCount);
    if (InputSystem::GetInputState("MouseLeft", InputStateMask::Trigger) && itemCount > 0)
    {
        // �`���[�W�̉����Đ�����
        beamChargeAudioComponent->Play(XAUDIO2_LOOP_INFINITE);

        DirectX::XMFLOAT3 pos = skeltalMeshComponent->GetJointWorldPosition("beam_FK");

        // �G�t�F�N�g�R���|�[�l���g�ɓ`�B
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

// �r�[���𔭎˂���֐�
void Player::FireBeam()
{
    // �`���[�W�̉����~����
    beamChargeAudioComponent->Stop();
    // ���ˉ����Đ�����
    beamLaunchAudioComponent->Play();

    float beamItemPower = static_cast<float>(rightItemCount + leftItemCount);
    int itemCount = rightItemCount + leftItemCount;

    char debugBuffer[128];
    sprintf_s(debugBuffer, sizeof(debugBuffer),
        "beamItemCount%d\n",
        itemCount);
    OutputDebugStringA(debugBuffer);

    //if (effectChargeComponent->GetEffectState() == EffectComponent::EffectState::Ending)
    {// ���߂��I�������A  Beam �𐶐�����
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
        // Beam �𐶐�����
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
        // ���݂̃v���C���[�X�e�[�g��\��
        //if (!stateStack.empty())
        //{
        //    std::vector<std::shared_ptr<PlayerState>> tempStack;
        //    std::stack<std::shared_ptr<PlayerState>> copyStack = stateStack; // �R�s�[���쐬

        //    // �X�^�b�N���x�N�^�[�ɃR�s�[
        //    while (!copyStack.empty())
        //    {
        //        tempStack.push_back(copyStack.top());
        //        copyStack.pop();
        //    }
        //    // �X�^�b�N�̏��Ԃ��t�ɂ��ĕ\���iTop���ŏ�ʂɂȂ�悤�Ɂj
        //    ImGui::Text("Player States:");
        //    for (auto it = tempStack.rbegin(); it != tempStack.rend(); ++it)
        //    {
        //        ImGui::Text("- %s", (*it)->GetStateName().c_str()); // �X�e�[�g����\��
        //    }
        //}
        //else
        //{
        //    ImGui::Text("No Active State");
        //}
        //DirectX::XMFLOAT3 position = GetPosition();
        //ImGui::DragFloat3("Position", &position.x, 0.5f);
        //���͏����擾
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

// �x���X�V����
void Player::LateUpdate(float elapsedTime)
{

}

void Player::Turn(float elapsedTime)
{
    using namespace DirectX;
#if 0
    //pad.Acquire();

//���͏����擾
    float thumbStateLx = pad.ThumbStateLx();
    float thumbStateLy = pad.ThumbStateLy();


    DirectX::XMVECTOR PAD = XMVectorSet(thumbStateLx, 0.0f, thumbStateLy, 0.0f);
    PAD = DirectX::XMVector3Normalize(PAD);
    // ���͂��قڃ[���Ȃ��]���Ȃ�
    if (std::abs(thumbStateLx) < 0.1f && std::abs(thumbStateLy) < 0.1f)
    {
        return;
    }

    //�v���C���[�������Ă���O�����x�N�g�������߂�
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //�v���C���[�̂R�������߂�
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////�J�����̒����_�ƈʒu���擾����
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////�J�����̍��W�������߂�
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    //const float inputDeadZone = 0.0001f;

    ////�X�e�B�b�N�̓��͂��J�����̍��W���ɕϊ����Đi�s�����̃x�N�g�������߂�
    ////�J������̈ړ�����
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    ////�i�s�����x�N�g����player�̍��̌����Ă�������� X ���E�x�N�g���œ��ς���
    //float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(CameraBasedMoveDirection, PX));

    //if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    //{
    //    turningSpeed = maxTurningSpeed * turningSpeedFactor;
    //}
    // 


    //float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(CameraBasedMoveDirection, PZ));
    //if (cross > 0.0f)
    //{//���ɉ�]����
    //    rotation.y -= rot;
    //}
    //else
    //{//�E�ɉ�]����
    //    rotation.y += rot;
    //}

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //�N�H�[�^�j�I�������]�s����쐬
    XMVECTOR Y = R.r[1];//���f���̃��[�J����Y�������o��
    Q = XMQuaternionNormalize(XMQuaternionMultiply(Q, XMQuaternionRotationAxis(Y, +DirectX::XMConvertToRadians(turningSpeed) * elapsedTime)));
    XMStoreFloat3(&rotation, Q);
#else
    //rotation.y += DirectX::XMConvertToRadians(turningSpeed) * elapsedTime;
#endif

#endif // 0
    float vx = inputComponent->GetThumbStateRx();
    float vz = inputComponent->GetThumbStateRy();
    float speed = DirectX::XMConvertToRadians(currentTurnSpeed) * elapsedTime;

    // �i�s�x�N�g�����[���x�N�g���̏ꍇ�͏�������K�v�Ȃ�
    float length = sqrtf(vx * vx + vz * vz);
    if (length <= 0.001f)
    {
        return;
    }
    // �i�s�x�N�g����P�ʃx�N�g����
    vx /= length;
    vz /= length;

    // ���g�̉�]�l����O���������߂�
    float frontX = sinf(angle.y);
    float frontZ = cosf(angle.y);

    // ��]�p�����߂邽�߁A2�̒P�ʃx�N�g���̓��ς��v�Z����
    float dot = frontX * vx + frontZ * vz;

    // ���ϒl��-1.0�`1.0�ŕ\������Ă���A2�̒P�ʃx�N�g���̊p�x��
    // �������ق�1.0�ɋ߂Â��Ƃ��������𗘗p���ĉ�]���x�𒲐�����
    float rot = 1.0f - dot;

    if (rot > speed)
    {
        rot = speed;
    }

    // ���E������s�����߂ɂQ�̒P�ʃx�N�g���̊O�ς��v�Z����
    float cross = (frontX * vz) - (frontZ * vx);

    // 2D�̊O�ϒl�����̏ꍇ�����̏ꍇ���ɂ���č��E���肪�s����
    // ���E������s�����Ƃɂ���č��E��]��I������
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
    // ���͂��قڃ[���Ȃ��]���Ȃ�
    if (std::abs(thumbStateRx) < 0.1f && std::abs(thumbStateRy) < 0.1f)
    {
        return;
    }

    //�v���C���[�������Ă���O�����x�N�g�������߂�
    front = GetForward();
    DirectX::XMVECTOR FRONT = DirectX::XMLoadFloat3(&front);
    FRONT = DirectX::XMVector3Normalize(FRONT);

    //�v���C���[�̂R�������߂�
    DirectX::XMVECTOR PX, PY, PZ;
    PY = DirectX::XMVectorSet(0, 1, 0, 0);
    PZ = FRONT;
    PX = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(PY, PZ));

    ////�J�����̒����_�ƈʒu���擾����
    //Camera& camera = Camera::Instance();
    //DirectX::XMFLOAT4 cameraPosition = camera.GetCameraPosition();
    //DirectX::XMFLOAT4 cameraFocus = camera.GetFocus();
    //XMVECTOR CP = XMLoadFloat4(&cameraPosition);
    //XMVECTOR CF = XMLoadFloat4(&cameraFocus);
    ////�J�����̍��W�������߂�
    //XMVECTOR CX, CY, CZ;
    //CY = XMVectorSet(0, 1, 0, 0);
    //CZ = XMVector3Normalize(CF - CP);
    //CX = XMVector3Normalize(XMVector3Cross(CY, CZ));
    //CY = XMVector3Normalize(XMVector3Cross(CZ, CX));

    const float inputDeadZone = 0.0001f;

    ////�X�e�B�b�N�̓��͂��J�����̍��W���ɕϊ����Đi�s�����̃x�N�g�������߂�
    ////�J������̈ړ�����
    //DirectX::XMVECTOR CameraBasedMoveDirection = DirectX::XMVector3Normalize(thumbStateLx * CX + thumbStateLy * CZ);

    DirectX::XMFLOAT3 moveDir = inputComponent->GetMoveInput();
    DirectX::XMVECTOR MoveDirection = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&moveDir));
    //float lenSq = moveDir.x * moveDir.x + moveDir.y * moveDir.y + moveDir.z * moveDir.z;
    //if (lenSq > 0.0001f)
    //{
    //    rotationComponent->SetDirection(moveDir);
    //}

    ////�i�s�����x�N�g����player�̍��̌����Ă�������� X ���E�x�N�g���œ��ς���
    float turningSpeedFactor = DirectX::XMVectorGetX(DirectX::XMVector3Dot(MoveDirection, PX));

    if (std::fabsf(turningSpeedFactor) > inputDeadZone)
    {
        turningSpeed = maxTurningSpeed * turningSpeedFactor;
    }



    float cross = DirectX::XMVectorGetX(DirectX::XMVector3Cross(MoveDirection, PZ));
    if (cross > 0.0f)
    {//���ɉ�]����
        angle.y -= rot;
    }
    else
    {//�E�ɉ�]����
        angle.y += rot;
    }

#if 0
    XMVECTOR Q = XMVectorSet(rotation.x, rotation.y, rotation.z, 0.0f);
    XMMATRIX R = XMMatrixRotationQuaternion(Q); //�N�H�[�^�j�I�������]�s����쐬
    XMVECTOR Y = R.r[1];//���f���̃��[�J����Y�������o��
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
    //�i�s�����̃x�N�g���擾
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
    {//�Ȃ��Ȃ鎞
        stateStack.top()->Exit(*this);
        stateStack.pop();
    }
    if (!stateStack.empty())
    {//�c���Ă���X�e�[�g
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

//�����������̏���
void Player::Hit()
{
    hp -= 1;
}

//�X�e�B�b�N�̓��͒l����ړ��x�N�g�����擾
DirectX::XMFLOAT3 Player::GetMoveVec()
{
    //pad.Acquire();
#if 0
    //���͏����擾
    float ax = pad.ThumbStateLx();
    float ay = pad.ThumbStateLy();

    //�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
    Camera& camera = Camera::Instance();
    const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
    const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

    //�J�����E�����x�N�g��[X��]��XZ���ʂł̒P�ʃx�N�g���ɕϊ�
    float cameraRightX = cameraRight.x;
    float cameraRightZ = cameraRight.z;
    float cameraRightLength = std::sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
    if (cameraRightLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraRightX /= cameraRightLength;
        cameraRightZ /= cameraRightLength;
    }

    //�J�����̑O�����̃x�N�g��[Z��]��XZ�P�ʃx�N�g���ɕϊ�
    float cameraFrontX = cameraFront.x;
    float cameraFrontZ = cameraFront.z;
    float cameraFrontLength = std::sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
    if (cameraFrontLength > 0.0f)
    {
        //�P�ʃx�N�g����
        cameraFrontX /= cameraFrontLength;
        cameraFrontZ /= cameraFrontLength;
    }

    //�i�s�x�N�g�����v�Z����
    DirectX::XMFLOAT3 vec;
    vec.x = (cameraRightX * ax) + (cameraFrontX * ay);
    vec.z = (cameraRightZ * ax) + (cameraFrontZ * ay);
    //Y�������ɂ͈ړ����Ȃ�
    vec.y = 0.0f;

    return vec;
#endif
    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

