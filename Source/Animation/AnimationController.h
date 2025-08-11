#ifndef ANIMATION_CONTROLLER_H
#define ANIMATION_CONTROLLER_H

// C++ 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクトの他のヘッダ
#include "Components/Render/MeshComponent.h"
#include "Graphics/Resource/InterleavedGltfModel.h"

// アニメーションのコントローラー  
class AnimationController
{
public:
    AnimationController(SkeltalMeshComponent* target) :target_(target)
    {
        // アニメーションブレンドに使用するノード
        animationNodes[0] = target_->model->GetNodes();
        animationNodes[1] = target_->model->GetNodes();

        blendAnimationNodes = target_->model->GetNodes();
    }

    void AddAnimation(std::string animationName, size_t animationClip)
    {
        animationNameToIndex_[animationName] = animationClip;
    }

    // アニメーション再生しているかどうか
    bool IsPlayAnimation()
    {
        return !(this->isAnimationFinished);
    }

    // 使用例
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
        { // ブレンドしないなら現在のアニメーションを次のアニメーションに変更する
            this->animationClip = animationNameToIndex_[animationName];
            transitionState = AnimationController::AnimationTransitionState::Completed;
        }
    }

    void OnUpdate(float deltaTime)
    {
        animationTime += deltaTime * animationRate;

        if (target_->model->animations.size() == 0)
        {// アニメーションがないモデルの場合
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
                blendFactor = animationTime / transitionTime;     //ゼロ除算を防ぐため
            }
            else
            {
                blendFactor = 1.0f;
            }
            target_->model->BlendAnimations(animationNodes[0], animationNodes[1], blendFactor, blendAnimationNodes);
            if (blendFactor >= 1.0f)
            {
                // 遷移終了
                transitionState = AnimationTransitionState::Completed;
                animationTime = 0.0f;
                // 現在のアニメーションクリップを次のアニメーションクリップに変更する
                this->animationClip = this->animationNextClip;
                //isBlendingAnimation = false;
            }
            break;
        case AnimationController::AnimationTransitionState::Completed:
            // 終わったら通常時に戻す
            if (target_->model->animations.at(animationClip).duration < animationTime)
            {
                if (isAnimationLoop)
                {//アニメーションをループするとき
                    if (requestStopLoop)
                    {
                        isAnimationLoop = false;    // ループしないモードにする
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
        // 描画に使うノードをブレンドのノードにする
        //target_->model->nodes = blendAnimationNodes;
        target_->modelNodes = blendAnimationNodes;
    }

    // アニメーションの再生倍率を変更する関数
    void SetAnimationRate(float animationRate) { this->animationRate = animationRate; }

    // アニメーションを止める処理
    void Stop()
    {
        isAnimationFinished = true;
        transitionState = AnimationTransitionState::NotStarted;
    }

    // アニメーションのループを切りよく終了させるフラグ
    void RequestStopLoop()
    {
        requestStopLoop = true;
    }
private:
    SkeltalMeshComponent* target_ = nullptr;

    std::unordered_map<std::string, size_t> animationNameToIndex_;

    // アニメーションブレンドに使用するノード
    std::vector<InterleavedGltfModel::Node> animationNodes[2];
    std::vector<InterleavedGltfModel::Node> blendAnimationNodes;

    enum class AnimationTransitionState
    {
        NotStarted,
        Inprogress,
        Completed,
    };

    //遷移ステート
    AnimationTransitionState transitionState = AnimationTransitionState::NotStarted;

    //アニメーションの再生倍率
    float animationRate = 1.0f;     //デフォルト 1,0f

    //アニメーション時間
    float animationTime = 0.0f;

    // 今再生しているアニメーションのインデックス
    size_t animationClip = 0;

    // 次再生したいアニメーションのインデックス
    size_t animationNextClip = 0;

    // アニメーションをループするか
    bool isAnimationLoop = true;

    // 現在のブレンドの比率
    float blendFactor = 0.0f;

    // ブレンド中かどうか
    bool isBlendingAnimation = false;

    // ブレンドしている時間
    float transitionTime = 0.0f;

    // アニメーションが終了したかどうか
    bool isAnimationFinished = false;

    // ループ終了フラグ 
    bool requestStopLoop = false; // 切りよくループを終わらせる

    // 今再生しているアニメーションの名前
    std::string currentAnimationName = "";
};

#endif  //ANIMATION_CONTROLLER_H