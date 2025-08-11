#ifndef SCENE_COMPONENT_H
#define SCENE_COMPONENT_H

// C++ 標準ライブラリ
#include <memory>
#include <string>
#include <vector>

// 他ライブラリ
#include <DirectXMath.h>

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

// プロジェクトの他のヘッダ
#include "Component.h"
#include "Components/Transform/Transform.h"


class Actor;


class SceneComponent :public Component, public std::enable_shared_from_this<SceneComponent>
{
public:
    SceneComponent(const std::string& name,std::shared_ptr<Actor> owner) :Component(name, owner)
    {
    }

    virtual ~SceneComponent() {}

    // 押し出された Transform を保存
    void SetPhysicalTransform(const Transform& t) 
    {
        physicalTransform_ = t; 
        hasPhysicalCorrection_ = true;
    }

    // Tick で最終Transformを取得するときはこれを使う
    Transform GetFinalWorldTransform() const
    {
        if (hasPhysicalCorrection_)
            return physicalTransform_;
        return componentToWorld_;
    }

    void ClearPhysicalCorrection()
    {
        hasPhysicalCorrection_ = false;
    }

    // 消去処理
    virtual void Destroy() override;

    // 初期化時に困るから即時 Transform 更新処理
    void UpdateTransformImmediate();
private:
    // SceneComponent や Transform 持ちクラスに追加
    DirectX::XMFLOAT3 inspectorEuler_ = { 0,0,0 };
    bool inspectorEulerInitialized_ = false;

    // 衝突押し出しで得る Transform を保存する用の変数
    Transform physicalTransform_;

    // このコンポーネントのワールド空間上でのTransform
    // final Transform_　親子関係を全て考慮した最終的なTransform
    Transform componentToWorld_;     // キャッシュ

    // 衝突押出の Transform が入っているか
    bool hasPhysicalCorrection_ = false;
protected:
    // 現在接続している親。　valid　なら　relativeLocation_ などはこのオブジェクトに対する相対値になる
    std::weak_ptr<SceneComponent> attachParent_; // 弱参照

    // 親のソケットノード (特定の接続ポイント) に接続する場合に使用されるオプションのインデックス
    int attachSocketNode_ = -1;

    // このコンポーネントに接続されている子供のシーンコンポーネントのリスト
    std::vector<std::shared_ptr<SceneComponent>> attachChildren_; // 子を保持（共有）

public:
    std::shared_ptr<SceneComponent> GetAttachParent() const
    {
        return attachParent_.lock();
    }

    int GetAttachSocketNode() const
    {
        return attachSocketNode_;
    }

    const std::vector<std::shared_ptr<SceneComponent>>& GetAttachChildren() const
    {
        return attachChildren_;
    }

    void DrawImGuiInspector() override
    {
#ifdef USE_IMGUI
        inspectorEuler_ = MathHelper::QuaternionToEuler(relativeRotation_);

#if 1
        if (ImGui::TreeNode((name_ + "  Transform").c_str()))
        {
            ImGui::DragFloat3("Relative Location", &relativeLocation_.x, 0.1f);
#if 0
            //if (!inspectorEulerInitialized_)
//{
//    inspectorEulerInitialized_ = true;
//}
//inspectorEuler_ = GetComponentEulerRotation();
            if (ImGui::DragFloat3("Relative Rotation", &inspectorEuler_.x, 1.0f))
            {
                DirectX::XMFLOAT3 eulerRad =
                {
                    DirectX::XMConvertToRadians(inspectorEuler_.x),
                    DirectX::XMConvertToRadians(inspectorEuler_.y),
                    DirectX::XMConvertToRadians(inspectorEuler_.z)
                };
                DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(eulerRad.x, eulerRad.y, eulerRad.z);
                DirectX::XMFLOAT4 q;
                DirectX::XMStoreFloat4(&q, quat);
                //SetWorldRotationDirect(q);
                SetRelativeRotationDirect(q);
            }

#endif // 0
            testAngle = GetRelativeEulerRotation();
            testAngle.x = DirectX::XMConvertToDegrees(testAngle.x);
            testAngle.y = DirectX::XMConvertToDegrees(testAngle.y);
            testAngle.z = DirectX::XMConvertToDegrees(testAngle.z);
            ImGui::DragFloat3("RelativeAngle", &testAngle.x, 1.0f);
            //testAngle.x = MathHelper::ClampAngle(testAngle.x);
            //testAngle.y = MathHelper::ClampAngle(testAngle.y);
            //testAngle.z = MathHelper::ClampAngle(testAngle.z);
            DirectX::XMFLOAT3 eulerRadNew =
            {
                DirectX::XMConvertToRadians(testAngle.x),
                DirectX::XMConvertToRadians(testAngle.y),
                DirectX::XMConvertToRadians(testAngle.z)
            };

            DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw(
                eulerRadNew.x, eulerRadNew.y, eulerRadNew.z
            );

            DirectX::XMFLOAT4 qNew;
            XMStoreFloat4(&qNew, quatNew);
            SetRelativeRotationDirect(qNew);
            //SetQuaternionRotation(qNew);

            //// UIで回転角度変更があった場合は反映
            //if (ImGui::DragFloat3("Relative Rotation", &inspectorEuler_.x, 1.0f))
            //{
            //    DirectX::XMFLOAT3 eulerRadNew = {
            //        DirectX::XMConvertToRadians(inspectorEuler_.x),
            //        DirectX::XMConvertToRadians(inspectorEuler_.y),
            //        DirectX::XMConvertToRadians(inspectorEuler_.z)
            //    };

            //    DirectX::XMVECTOR quatNew = DirectX::XMQuaternionRotationRollPitchYaw(
            //        eulerRadNew.x, eulerRadNew.y, eulerRadNew.z
            //    );

            //    DirectX::XMFLOAT4 qNew;
            //    XMStoreFloat4(&qNew, quatNew);
            //    SetRelativeRotationDirect(qNew);
            //}
            ImGui::DragFloat3("Relative Scale", &relativeScale_.x, 0.01f, 0.01f, 100.0f);
            ImGui::TreePop();
        }


        if (ImGui::TreeNode((name_ + "   Debug Info").c_str()))
        {
            ImGui::Text("Children: %zu", attachChildren_.size());
            ImGui::Text("Parent: %s", attachParent_.lock() ? attachParent_.lock()->name().c_str() : "None");
            ImGui::TreePop();
        }
#endif
#endif
    }
private:

    // 親からの相対的な位置
    DirectX::XMFLOAT3 relativeLocation_ = { 0.0f,0.0f,0.0f };

    // 親からの相対的なクォータニオン
    DirectX::XMFLOAT4 relativeRotation_ = { 0.0f,0.0f,0.0f,1.0f };

    // 親からの相対的なスケール
    DirectX::XMFLOAT3 relativeScale_ = { 1.0f,1.0f,1.0f };

private:
    // 0はfalse 1はtrue 符号なし8ビット整数

    // 相対的な位置、回転、スケールに基づいて worldTransform　を更新したことがあれば true
    // 開始時に　worldTrnasform が初期化されているかどうかを確認するために使う
    uint8_t componentToWorldTransformUpdate_ : 1 = 0; // <-１ビットだけ使う 

    // trueの時に、このコンポーネントやその子に対して、updateOverlapsを呼び出す必要がない
    // これは、ツリーを辿っても当たり判定の更新が不要な場合に、パフォーマンス最適化として使われる
    // 通常このフラグは　UpdateOverlaps 実行後に true　にセットされる
    // 状態（アタッチ変更、当たり判定設定など）が変わったときは、clearSkipUpdateOverlaps() を呼んでフラグを無効化する。
    uint8_t skipUpdateOverlaps_ : 1 = 0; //→ 当たり判定の更新処理をスキップして高速化するためのフラグ
    // 子コンポーネントに影響がない場合、無駄な処理を避ける

//　通常、シーンコンポーネントの位置などは親からの相対座標だけど、
// これらフラグがtrueだとワールドに対して直接指定される
// relativeLocation_ を親ではなくワールド座標系に対する位置とみなす場合に true
    uint8_t absoluteLocation_ : 1 = 0;
    // relativeRotation_ を親ではなくワールド座標系に対する位置とみなす場合に true
    uint8_t absoluteRotation_ : 1 = 0;
    // relativeScale_ を親ではなくワールド座標系に対する位置とみなす場合に true
    uint8_t absoluteScale_ : 1 = 0;

    // true の場合はこのコンポーネントは描画されかつ、影も落とす
    // false の場合は描画もされず、影も落とさない
    uint8_t visible_ : 1 = 1;

public:
    bool IsUsingAbsoluteLocation() const
    {
        return absoluteLocation_;
    }
    void SetUsingAbsoluteLocation(bool absoluteLocation)
    {
        absoluteLocation_ = absoluteLocation ? 1 : 0;
    }
    bool IsUsingAbsoluteRotation() const
    {
        return absoluteRotation_;
    }
    void SetUsingAbsoluteRotation(bool absoluteRotation)
    {
        absoluteRotation_ = absoluteRotation ? 1 : 0;
    }
    bool IsUsingAbsoluteScale() const
    {
        return absoluteScale_;
    }
    void SetUsingAbsoluteScale(bool absoluteScale)
    {
        absoluteScale_ = absoluteScale ? 1 : 0;
    }


public:
    // 相対的な座標を取得
    DirectX::XMFLOAT3 GetRelativeLocation() const
    {
        return relativeLocation_;
    }
    // 直接　相対的な座標を設定
    void SetRelativeLocationDirect(const DirectX::XMFLOAT3& newRelativeLocation)
    {
        relativeLocation_ = newRelativeLocation;
    }
    // 相対的なスケールを取得
    DirectX::XMFLOAT3 GetRelativeScale()const
    {
        return relativeScale_;
    }
    // 直接　相対的なスケールを設定
    void SetRelativeScaleDirect(const DirectX::XMFLOAT3& newRelativeScale)
    {
        relativeScale_ = newRelativeScale;
    }
    // 相対的な角度を取得
    DirectX::XMFLOAT3 GetRelativeEulerRotation()const
    {
        DirectX::XMFLOAT3 angle= MathHelper::QuaternionToEuler(relativeRotation_);
        return angle;
    }
    DirectX::XMFLOAT4 QuaternionFromEulerYXZ(const DirectX::XMFLOAT3& eulerRadians)
    {
        using namespace DirectX;
        XMVECTOR qx = XMQuaternionRotationAxis({ 1,0,0 }, eulerRadians.x);
        XMVECTOR qy = XMQuaternionRotationAxis({ 0,1,0 }, eulerRadians.y);
        XMVECTOR qz = XMQuaternionRotationAxis({ 0,0,1 }, eulerRadians.z);

        // 順序 Y → X → Z（YXZ intrinsic）
        XMVECTOR q = XMQuaternionMultiply(qz, XMQuaternionMultiply(qx, qy));
        XMFLOAT4 result;
        XMStoreFloat4(&result, q);
        return result;
    }
    // 直接　相対的な角度を設定
    void SetRelativeEulerRotationDirect(const DirectX::XMFLOAT3& newEulerRotaion)
    {
        DirectX::XMStoreFloat4(&relativeRotation_, DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(newEulerRotaion.x), DirectX::XMConvertToRadians(newEulerRotaion.y), DirectX::XMConvertToRadians(newEulerRotaion.z)));
    }
    // 相対的なクォータニオンを取得
    const DirectX::XMFLOAT4& GetRelativeRotation() const
    {
        return relativeRotation_;
    }
    // 直接　相対的なクォータニオンを設定
    void SetRelativeRotationDirect(const DirectX::XMFLOAT4& newRelativeRotation)
    {
        relativeRotation_ = newRelativeRotation;
    }
    //  このコンポーネントのワールド空間上でのTransformを取得
    const Transform& GetComponentWorldTransform() const
    {
        return componentToWorld_;
    }
    //  このコンポーネントのワールド空間上でのTransformを取得(書き換え用)
    Transform& GetComponentWorldTransform()
    {
        return componentToWorld_;
    }
    // ワールド空間でのこのコンポーネントの位置を取得
    DirectX::XMFLOAT3 GetComponentLocation() const
    {
        return GetComponentWorldTransform().GetLocation();
    }
    // ワールド空間でのこのコンポーネントのクォータニオンを取得
    DirectX::XMFLOAT4 GetComponentRotation() const
    {
        return GetComponentWorldTransform().GetRotation();
    }
    // ワールド空間でのこのコンポーネントのスケールを取得
    DirectX::XMFLOAT3 GetComponentScale() const
    {
        return GetComponentWorldTransform().GetScale();
    }
    // ワールド空間でのこのコンポーネントの角度を取得
    DirectX::XMFLOAT3 GetComponentEulerRotation() const
    {
        return GetComponentWorldTransform().GetEulerRotation();
    }
    // ワールド空間でのこのコンポーネントの座標を設定
    // （親がある場合は、親のワールド行列を使って相対位置に変換してセットする）
    virtual void SetWorldLocationDirect(const DirectX::XMFLOAT3& newWorldLocation)
    {
#if 0
        GetComponentWorldTransform().SetTranslation(newWorldLocation);
#else
        if (auto parent = attachParent_.lock())
        {
            // 親のワールドトランスフォームの逆行列を使って、相対位置を求める
#if 1
            DirectX::XMMATRIX parentWorld = parent->GetComponentWorldTransform().ToMatrix();
            DirectX::XMMATRIX parentInv = DirectX::XMMatrixInverse(nullptr, parentWorld);
            DirectX::XMVECTOR worldPos = DirectX::XMLoadFloat3(&newWorldLocation);
            DirectX::XMVECTOR localPos = DirectX::XMVector3TransformCoord(worldPos, parentInv);
            DirectX::XMFLOAT3 relative;
            DirectX::XMStoreFloat3(&relative, localPos);
            SetRelativeLocationDirect(relative);
#else
            parent->SetRelativeLocationDirect(newWorldLocation);
#endif
        }
        else
        {
            SetRelativeLocationDirect(newWorldLocation);
        }
#endif
    }
    // ワールド空間でのこのコンポーネントのクォータニオンを設定
    void SetWorldRotationDirect(const DirectX::XMFLOAT4& newWorldRotation)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeRotationDirect(newWorldRotation);
        }
        else
        {
            SetRelativeRotationDirect(newWorldRotation);
        }
    }
    // ワールド空間でのこのコンポーネントの角度を設定
    void SetWorldEulerRotationDirect(const DirectX::XMFLOAT3& newWorldEuler)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeEulerRotationDirect(newWorldEuler);
        }
        else
        {
            SetRelativeEulerRotationDirect(newWorldEuler);
        }
    }
    // ワールド空間でのこのコンポーネントのスケールを設定
    void SetWorldScaleDirect(const DirectX::XMFLOAT3& newWorldScale)
    {
        if (auto parent = attachParent_.lock())
        {
            parent->SetRelativeScaleDirect(newWorldScale);
        }
        else
        {
            SetRelativeScaleDirect(newWorldScale);
        }
    }



    // このコンポーネントが動いた時にコールバック(呼び出)される関数
    virtual void OnUpdateTransform(UpdateTransformFlags updateTransformFlags, TeleportType teleport = TeleportType::None)
    {
    }

    // コンポーネントの worldTransform の　update が　false だったら　worldTrasnformUpdate を呼ぶ
    void ConditionalUpdateComponentWorldTransform()
    {
        if (!componentToWorldTransformUpdate_)
        {
            UpdateComponentToWorld();
        }
    }

    // このコンポーネントにアタッチされている全ての子コンポーネントたちの Transform を更新する
    void UpdateChildTransforms(UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None);

protected:
    // このコンポーネントの親からの相対的な Transform を返す
    Transform GetRelativeTransform() const
    {
        return Transform(relativeLocation_, relativeRotation_, relativeScale_);
    }

    // 指定されたソケットノードのワールド空間のTransformを返す
    // ソケットが見つからなかった場合は、自身の WorldTransform を返す
    virtual Transform GetSocketTransform(int socketNode) const
    {
        // TODO: // 自身のワールド空間のトランスフォーム

        return componentToWorld_;
    }

    // このコンポーネントの新しい　componentToWorld_Transformを計算する
    // parent は省略可能で、任意の sceneComponent を使って計算できる
    // 指定されない場合は、コンポーネント自身の　attachParent を使う
    Transform CalculateNewComponentToWorldTransform(const Transform& newRelativeTransform, const SceneComponent* parent = nullptr, int socketNode = -1)const
    {
        // socketNode と parent を指定されていなければ、アタッチされた情報から取得
        socketNode = parent ? socketNode : attachSocketNode_;
        parent = parent ? parent : attachParent_.lock().get();
        if (parent)
        {
            // 「絶対位置・絶対回転・絶対スケール」のいずれかを使っているか
            const bool general = IsUsingAbsoluteLocation() || IsUsingAbsoluteRotation() || IsUsingAbsoluteScale();
            if (!general)
            {// 絶対座標指定がされていないなら（＝通常の親からの相対的な座標なら）
                return newRelativeTransform * parent->GetSocketTransform(socketNode);
            }
            // 絶対指定が含まれているから、特殊な合成処理を行う
            return CalculateNewComponentToWorldGeneralCase(newRelativeTransform, parent, socketNode);
        }
        else
        {// 親が存在しない時は、自分の相対 Transform がそのまま WorldTransform になる
            return newRelativeTransform;
        }
    }

    // 親の Transform を使用して、絶対指定の要素を考慮した　componentToWorldTransform を計算する
    Transform CalculateNewComponentToWorldGeneralCase(const Transform& newRelativeTransform, const SceneComponent* parent, int socketNode)const
    {
        if (parent != nullptr)
        {
            const Transform parentToWorld = parent->GetSocketTransform(socketNode);
            Transform newComponentToWorldTransform = newRelativeTransform * parentToWorld;

            // 絶対位置が有効なら、位置だけは親の影響を無視
            if (absoluteLocation_)
            {
                newComponentToWorldTransform.translation_ = newRelativeTransform.translation_;
            }
            // 絶対回転が有効なら、回転だけは親の影響を無視
            if (absoluteRotation_)
            {
                newComponentToWorldTransform.rotation_ = newRelativeTransform.rotation_;
            }
            // 絶対スケールが有効なら、スケールだけは親の影響を無視
            if (absoluteScale_)
            {
                //newComponentToWorldTransform.scale_ = newRelativeTransform.scale_;
                // ただの代入ではなく、符号の補正を加えたスケール合成
                newComponentToWorldTransform.scale_ = DirectX::XMVectorMultiply(newRelativeTransform.scale_, MathHelper::VectorSign(newComponentToWorldTransform.scale_));
            }

            return newComponentToWorldTransform;
        }
        else
        {
            return newRelativeTransform;
        }
    }

    // このコンポーネントを、指定された親コンポーネントにアタッチ（接続）する
    // parent はアタッチ先の親コンポーネント　
    // socketNode 接続先のソケットノード番号( -1 ならデフォルト)
    void AttachToComponent(const std::shared_ptr<SceneComponent>& parent, int socketNode);

    // このコンポーネントを現在の親のコンポーネントから切り離す
    void DetachFromParent();


private:
    //「Transformが変わった時や物理的に移動した時に、自分とその子たちを正しく更新するための中心的な処理」
    // 自身の Transform の変更を反映し、必要に応じて子コンポーネントにもそれを伝播させる関数
    void PropagateTransformUpdate(bool transformChanged, UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None);

    // 親の Transform をもとに、自分の Transform（= ComponentToWorld）を更新し、必要に応じてその変更を子へ伝播する
    void UpdateComponentToWorldWithParent(SceneComponent* parent, int socketNode, UpdateTransformFlags updateTransformFlags, TeleportType teleport);


    //  指定された component が、このコンポーネントの子孫（子、孫など）であるかを判定する関数。
    // component が自分自身または子孫であれば true を返して、それ以外は false を返す
    bool IsAttachBelow(SceneComponent* component)
    {
        // 自分自身と一致する場合は true（＝component は自分、よって子孫関係にある）
        if (component == this)
        {
            return true;
        }

        // 子コンポーネントそれぞれに対して、再帰的に子孫関係をチェック
        for (const std::shared_ptr<SceneComponent>& child : attachChildren_)
        {
            if (child->IsAttachBelow(component))   // 再帰的チェック
            {
                return true;
            }
        }

        // どの子孫にも該当しなければ false
        return false;
    }

    // 指定された component が、このコンポーネントの祖先（親、祖父母など）であるかを判定する関数。
    // component が祖先であれば true を返し、それ以外は false を返す。
    bool IsAttachAbove(SceneComponent* component)
    {
        // 親から辿っていって、指定された component に一致するかをチェック
        for (SceneComponent* parent = attachParent_.lock().get(); parent; parent = parent->attachParent_.lock().get())
        {
            if (parent == component)
            {
                return true;
            }
        }

        // どの親にも一致しなければ false
        return false;
    }

    friend class Actor;

public:
    virtual void UpdateComponentToWorld(UpdateTransformFlags updateTransformFlags = UpdateTransformFlags::None, TeleportType teleport = TeleportType::None) override final;

    virtual void Tick(float deltaTime) {}


    virtual void Initialize()override {};

    virtual void OnRegister()override {} // 派生クラスで override して登録処理を書く

    virtual void OnUnregister()override {} // 派生クラスで override して解除処理を書く


#if 0
    void SetLerpQuaternion(DirectX::XMFLOAT3 angle)
    {
        DirectX::XMFLOAT3 euler = { DirectX::XMConvertToRadians(angle.x),DirectX::XMConvertToRadians(angle.y),DirectX::XMConvertToRadians(angle.z) };
        DirectX::XMVECTOR q = DirectX::XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z);
        DirectX::XMStoreFloat4(&afterRotation, q);
        if (std::abs(beforeRotation.y - afterRotation.y) <= FLT_EPSILON)
        {// 前のrotaionと変更後のrotationが一緒の場合スキップする
            return;
        }
        beforeRotation = rotationLocal;
        lerpTime = 0.0f;
    }

    void LerpQuaternion(float deltaTime)
    {
        DirectX::XMVECTOR qAfter = DirectX::XMLoadFloat4(&afterRotation);
        DirectX::XMVECTOR qBefore = DirectX::XMLoadFloat4(&beforeRotation);
        lerpTime += deltaTime * 0.8f;
        if (lerpTime > 1.0f)
        {
            lerpTime = 1.0f;
        }
        DirectX::XMVECTOR q = DirectX::XMQuaternionSlerp(qBefore, qAfter, lerpTime);
        DirectX::XMStoreFloat4(&rotationLocal, q);
    }

#endif

    // 親子関係セット
    void AttachTo(const std::shared_ptr<SceneComponent>& parent)
    {
        attachParent_ = parent;
        parent->attachChildren_.push_back(shared_from_this());
        // もう一度確認
    }
    void AddWorldOffset(const DirectX::XMFLOAT3& offset);

    bool isDirty = true;


    // テストのちに削除
    DirectX::XMFLOAT4 afterRotation = { 0.0f,0.0f,0.0f,1.0f }; // クォータニオン
    DirectX::XMFLOAT4 beforeRotation = { 0.0f,0.0f,0.0f,1.0f }; // クォータニオン
    float lerpTime = 0.0f;


    // テスト
    DirectX::XMFLOAT3 testAngle = { 0.0f,0.0f,0.0f };
};


#endif  //SCENE_COMPONENT_H