#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

// C++ �W�����C�u����
#include <string>
#include <unordered_map>
#include <vector>

// �v���W�F�N�g�̑��̃w�b�_
#include "Components/Render/MeshComponent.h"
#include "Graphics/Resource/InterleavedGltfModel.h"

// �A�j���[�V�����̃R���g���[���[  
class AnimationController
{
public:
    AnimationController(SkeltalMeshComponent* target) :target_(target)
    {
        // �A�j���[�V�����u�����h�Ɏg�p����m�[�h
        animationNodes[0] = target_->model->GetNodes();
        animationNodes[1] = target_->model->GetNodes();

        blendAnimationNodes = target_->model->GetNodes();
    }

    void AddAnimation(std::string animationName, size_t animationClip)
    {
        animationNameToIndex_[animationName] = animationClip;
    }

    // �A�j���[�V�����Đ����Ă��邩�ǂ���
    bool IsPlayAnimation()
    {
        return !(this->isAnimationFinished);
    }

    // �g�p��
    // modelComponent->SetAnimationClip(
    void SetAnimationClip(/*size_t animationClip,*/std::string animationName, bool loop = false, bool isBlend = false, float blendTime = 0.3f)
    {
        this->isAnimationFinished = false;
        this->animationNextClip = animationNameToIndex_[animationName];
        this->isAnimationLoop = loop;
        this->currentAnimationName = animationName;
        this->transitionTime = blendTime;
        if (isBlend)
        {
            transitionState = AnimationController::AnimationTransitionState::NotStarted;
        }
        else
        { // �u�����h���Ȃ��Ȃ猻�݂̃A�j���[�V���������̃A�j���[�V�����ɕύX����
            this->animationClip = animationNameToIndex_[animationName];
            transitionState = AnimationController::AnimationTransitionState::Completed;
        }
    }

    void OnUpdate(float deltaTime)
    {
        animationTime += deltaTime * animationRate;

        if (target_->model->animations.size() == 0)
        {// �A�j���[�V�������Ȃ����f���̏ꍇ
            return;
        }

        if (isBlendingAnimation && transitionTime > 0.0f)
        {

        }

        switch (transitionState)
        {
        case AnimationController::AnimationTransitionState::NotStarted:
            target_->model->Animate(this->animationClip, animationTime, animationNodes[0]);
            target_->model->Animate(this->animationNextClip, 0.0f, animationNodes[1]);
            transitionState = AnimationTransitionState::Inprogress;
            animationTime = 0.0f;
            blendFactor = 0.0f;
            break;
        case AnimationController::AnimationTransitionState::Inprogress:
            if (transitionTime > 0.0f)
            {
                blendFactor = animationTime / transitionTime;     //�[�����Z��h������
            }
            else
            {
                blendFactor = 1.0f;
            }
            target_->model->BlendAnimations(animationNodes[0], animationNodes[1], blendFactor, blendAnimationNodes);
            if (blendFactor >= 1.0f)
            {
                // �J�ڏI��
                transitionState = AnimationTransitionState::Completed;
                animationTime = 0.0f;
                // ���݂̃A�j���[�V�����N���b�v�����̃A�j���[�V�����N���b�v�ɕύX����
                this->animationClip = this->animationNextClip;
                //isBlendingAnimation = false;
            }
            break;
        case AnimationController::AnimationTransitionState::Completed:
            // �I�������ʏ펞�ɖ߂�
            if (target_->model->animations.at(animationClip).duration < animationTime)
            {
                if (isAnimationLoop)
                {//�A�j���[�V���������[�v����Ƃ�
                    if (requestStopLoop)
                    {
                        isAnimationLoop = false;    // ���[�v���Ȃ����[�h�ɂ���
                        animationTime = 0.0f;   
                        requestStopLoop = false;
                    }
                    else
                    {
                        animationTime = 0;
                    }
                }
                else
                {
                    isAnimationFinished = true;
                }
            }
            target_->model->Animate(animationClip, animationTime, blendAnimationNodes);
            break;
        default:
            break;
        }
        // �`��Ɏg���m�[�h���u�����h�̃m�[�h�ɂ���
        //target_->model->nodes = blendAnimationNodes;
        target_->modelNodes = blendAnimationNodes;
    }

    // �A�j���[�V�����̍Đ��{����ύX����֐�
    void SetAnimationRate(float animationRate) { this->animationRate = animationRate; }

    // �A�j���[�V�������~�߂鏈��
    void Stop()
    {
        isAnimationFinished = true;
        transitionState = AnimationTransitionState::NotStarted;
    }

    // �A�j���[�V�����̃��[�v��؂�悭�I��������t���O
    void RequestStopLoop()
    {
        requestStopLoop = true;
    }
private:
    SkeltalMeshComponent* target_ = nullptr;

    std::unordered_map<std::string, size_t> animationNameToIndex_;

    // �A�j���[�V�����u�����h�Ɏg�p����m�[�h
    std::vector<InterleavedGltfModel::Node> animationNodes[2];
    std::vector<InterleavedGltfModel::Node> blendAnimationNodes;

    enum class AnimationTransitionState
    {
        NotStarted,
        Inprogress,
        Completed,
    };

    //�J�ڃX�e�[�g
    AnimationTransitionState transitionState = AnimationTransitionState::NotStarted;

    //�A�j���[�V�����̍Đ��{��
    float animationRate = 1.0f;     //�f�t�H���g 1,0f

    //�A�j���[�V��������
    float animationTime = 0.0f;

    // ���Đ����Ă���A�j���[�V�����̃C���f�b�N�X
    size_t animationClip = 0;

    // ���Đ��������A�j���[�V�����̃C���f�b�N�X
    size_t animationNextClip = 0;

    // �A�j���[�V���������[�v���邩
    bool isAnimationLoop = true;

    // ���݂̃u�����h�̔䗦
    float blendFactor = 0.0f;

    // �u�����h�����ǂ���
    bool isBlendingAnimation = false;

    // �u�����h���Ă��鎞��
    float transitionTime = 0.0f;

    // �A�j���[�V�������I���������ǂ���
    bool isAnimationFinished = false;

    // ���[�v�I���t���O 
    bool requestStopLoop = false; // �؂�悭���[�v���I��点��

    // ���Đ����Ă���A�j���[�V�����̖��O
    std::string currentAnimationName = "";
};

#endif  //ANIMATION_CONTROLLER_H