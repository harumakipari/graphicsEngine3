#include "ActorManager.h"
#include "Engine/Scene/Scene.h"


void Renderer::RenderParticle(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        std::vector<EffectComponent*> effectComponents;
        actor->GetComponents<EffectComponent>(effectComponents);

        for (const EffectComponent* effectComponent : effectComponents)
        {
        }
    }
}

void Renderer::RenderOpaque(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_ || !actor->isActive)
        {
            continue;
        }

        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        if (auto item = std::dynamic_pointer_cast<PickUpItem>(actor))
        {// �A�C�e���Ȃ�
            continue;
        }

        for (const MeshComponent* meshComponent : meshComponents)
        {
            //  �e MeshComponent ���g�̍ŐV���[���h�s������o��
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();

            if (meshComponent->IsVisible())
            {
                //  �`��Ăяo���� meshComponent �x�[�X�̍s���n��
                meshComponent->RenderOpaque(immediateContext, worldMat);
            }
        }
    }
}

void Renderer::RenderMask(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }
        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        if (auto item = std::dynamic_pointer_cast<PickUpItem>(actor))
        {// �A�C�e���Ȃ�
            continue;
        }

        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        for (const MeshComponent* meshComponent : meshComponents)
        {
            //  �e MeshComponent ���g�̍ŐV���[���h�s������o��
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            bool rendered = false;

            // ConvexCollisionComponent ���g�p�ł���Ȃ炻�����D�悷��
            if (auto* convexComponent = actor->GetComponent<ConvexCollisionComponent>())
            {
                if (convexComponent->GetActive())
                {
                    convexComponent = dynamic_cast<ConvexCollisionComponent*>(convexComponent);
                    DirectX::XMFLOAT4X4 world;
                    DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
                    convexComponent->GetMeshComponent()->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Mask);
                    //meshComponent->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Mask);
                    rendered = true;
                }
            }

            if (!rendered && meshComponent->IsVisible())
            {
                //  �`��Ăяo���� meshComponent �x�[�X�̍s���n��
                meshComponent->RenderMask(immediateContext, worldMat);
            }
        }
    }
}

void Renderer::RenderBlend(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        if (auto item = std::dynamic_pointer_cast<PickUpItem>(actor))
        {// �A�C�e���Ȃ�
            continue;
        }

        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);

        for (const MeshComponent* meshComponent : meshComponents)
        {
            //  �e MeshComponent ���g�̍ŐV���[���h�s������o��
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            bool rendered = false;

            // ConvexCollisionComponent ���g�p�ł���Ȃ炻�����D�悷��
            if (auto* convexComponent = actor->GetComponent<ConvexCollisionComponent>())
            {
                if (convexComponent->GetActive())
                {
                    convexComponent = dynamic_cast<ConvexCollisionComponent*>(convexComponent);
                    DirectX::XMFLOAT4X4 world;
                    DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
                    convexComponent->GetMeshComponent()->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Blend);
                    //meshComponent->model->Render(immediateContext, world, convexComponent->GetAnimatedNodes(), InterleavedGltfModel::RenderPass::Blend);
                    rendered = true;
                }
            }

            if (!rendered && meshComponent->IsVisible())
            {
                //  �`��Ăяo���� meshComponent �x�[�X�̍s���n��
                meshComponent->RenderBlend(immediateContext, worldMat);
            }
        }
    }
}

void Renderer::RenderInstanced(ID3D11DeviceContext* immediateContext)
{
    instanceDatas.clear();
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }
        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        if (auto item = std::dynamic_pointer_cast<PickUpItem>(actor))
        {// �A�C�e���Ȃ�
            if (item->skeltalMeshComponent->IsVisible())
            {// �A�C�e�����`�悳���Ȃ�
                DirectX::XMFLOAT4X4 instanceMatrix = item->GetWorldTransform();
                instanceDatas.push_back(instanceMatrix);
            }
        }
    }
    HRESULT hr{ S_OK };
    D3D11_MAPPED_SUBRESOURCE mapped_subresource{};
    hr = immediateContext->Map(itemModel->instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD,
        0, &mapped_subresource);
    _ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


    //DirectX::XMFLOAT4X4* p = reinterpret_cast<DirectX::XMFLOAT4X4*>(mapped_subresource.pData);

    memcpy_s(mapped_subresource.pData, sizeof(DirectX::XMFLOAT4X4) * 1000, instanceDatas.data(), sizeof(DirectX::XMFLOAT4X4) * instanceDatas.size());
    itemModel->instanceCount_ = static_cast<int>(instanceDatas.size());
    immediateContext->Unmap(itemModel->instanceBuffer.Get(), 0);
    itemModel->InstancedStaticBatchRender(immediateContext, InterleavedGltfModel::RenderPass::All, pipeLineState_);
    //char buf[256];
    //sprintf_s(buf, "instanceSize:%d\n", static_cast<int>(instanceDatas.size()));
    //OutputDebugStringA(buf);
}

void Renderer::CastShadowRender(ID3D11DeviceContext* immediateContext)
{
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        std::vector<MeshComponent*> meshComponents;
        actor->GetComponents<MeshComponent>(meshComponents);
        for (const MeshComponent* meshComponent : meshComponents)
        {
            //  �e MeshComponent ���g�̍ŐV���[���h�s������o��
            const auto& worldMat = meshComponent->GetComponentWorldTransform().ToWorldTransform();
            //bool rendered = false;

            // ConvexCollisionComponent ���g�p�ł���Ȃ炻�����D�悷��
            if (auto* convexComponent = actor->GetComponent<ConvexCollisionComponent>())
            {
                if (convexComponent->GetActive())
                {
                    convexComponent = dynamic_cast<ConvexCollisionComponent*>(convexComponent);
                    DirectX::XMFLOAT4X4 world;
                    DirectX::XMStoreFloat4x4(&world, DirectX::XMMatrixIdentity());
                    convexComponent->GetMeshComponent()->model->CastShadow(immediateContext, world, convexComponent->GetAnimatedNodes());

                    //meshComponent->model->CastShadow(immediateContext, world, convexComponent->GetAnimatedNodes());
                    //rendered = true;
                }
            }

            if (/*!rendered &&*/ meshComponent->IsVisible())
            {
                if (meshComponent->IsCastShadow())
                {
                    //  �`��Ăяo���� meshComponent �x�[�X�̍s���n��
                    meshComponent->CastShadow(immediateContext, worldMat);
                }
            }

        }
        if (auto build = actor->GetComponent<BuildMeshComponent>())
        {
            if (build->IsCastShadow())
            {
                const auto& worldMat = build->GetComponentWorldTransform().ToWorldTransform();
                build->CastShadow(immediateContext, worldMat);
            }
        }
    }
}

void ActorColliderManager::DebugRender(ID3D11DeviceContext* immediateContext)
{
    ShapeRenderer& shapeRenderer = ShapeRenderer::Instance();
    // �S�A�N�^�[����ShapeComponent���擾
    Scene* currentScene = Scene::GetCurrentScene();  // ���݂̃V�[���擾
    if (!currentScene) return;
    auto& allActors = currentScene->GetActorManager()->GetAllActors();
    for (auto& actor : allActors)
    {
        if (!actor->rootComponent_)
        {
            continue;
        }

        if (!actor->isActive)
        {// actor�����݂��Ă��Ȃ�������X�L�b�v
            continue;
        }

        std::vector<ShapeComponent*> shapeComponents;
        actor->GetComponents<ShapeComponent>(shapeComponents);

        for (const auto& shapeComponent : shapeComponents)
        {
            if (!shapeComponent->IsVisibleDebugBox())
            {
                continue;
            }
            //DirectX::XMFLOAT3 pos = shapeComponent->GetComponentLocation();
            //DirectX::XMFLOAT3 angle = shapeComponent->GetComponentEulerRotation();
            //DirectX::XMFLOAT3 size = shapeComponent->GetSizeFromAABB(shapeComponent->GetAABB());
            //shapeRenderer->DrawBox(immediateContext, shapeComponent->GetPosition(), shapeComponent->GetLocalAngle(), shapeComponent->GetSizeFromAABB(shapeComponent->GetAABB()), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
            //shapeRenderer->DrawBox(immediateContext, pos, angle, size, DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
            shapeRenderer.DrawBox(immediateContext, shapeComponent->GetComponentLocation(), shapeComponent->GetComponentEulerRotation(), shapeComponent->GetSizeFromAABB(shapeComponent->GetAABB()), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
        }
        //auto shape = actor->GetComponentByName("capsuleComponent");

        //DirectX::XMFLOAT3 pos = shape->GetComponentLocation();

        //DirectX::XMFLOAT3 pos = actor->GetComponentByName("capsuleComponent")->GetComponentLocation();
        //if (shape)
        {
            //DirectX::XMFLOAT3 pos = shape->GetComponentLocation();
            //shapeRenderer->DrawBox(immediateContext, pos, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
        }

        for (auto* shapeComponent : shapeComponents)
        {
            if (!shapeComponent->IsVisibleDebugShape())
            {
                continue;
            }
            if (auto sphere = dynamic_cast<SphereComponent*>(shapeComponent))
            {
                auto info = sphere->GetPhysicsShapeInfo();
                const physx::PxSphereGeometry& sphereGeo = info.geometry.sphere();
                const physx::PxTransform& trans = info.transform;
                // �ʒu
                DirectX::XMFLOAT3 pos(trans.p.x, trans.p.y, trans.p.z);

                shapeRenderer.DrawSphere(immediateContext, sphere->GetComponentLocation(), sphere->GetRadius(), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
                //shapeRenderer->DrawSphere(immediateContext, sphere->GetPosition(), sphere->GetRadius(), DirectX::XMFLOAT4(1.0f,0.0f,0.0f,1.0f));
                //shapeRenderer->DrawSphere(immediateContext, pos, sphereGeo.radius, DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            else if (auto capsule = dynamic_cast<CapsuleComponent*>(shapeComponent))
            {
#if 0
                shapeRenderer->DrawCapsule(immediateContext, capsule->GetPosition(), capsule->GetRadius(), capsule->GetHeight(), debugColor);
#else
                auto info = capsule->GetPhysicsShapeInfo();
                const physx::PxCapsuleGeometry& capsuleGeo = info.geometry.capsule();
                const physx::PxTransform& trans = info.transform;

                // �ʒu
                DirectX::XMFLOAT3 pos(trans.p.x, trans.p.y, trans.p.z);

                // ��]�iPxQuat �� XMFLOAT4�j
                DirectX::XMFLOAT4 rot(trans.q.x, trans.q.y, trans.q.z, trans.q.w);

                // �f�o�b�O�`��֐��ɉ�]��n����Ȃ�
                shapeRenderer.DrawCapsule(immediateContext, pos/*, rot*/, capsuleGeo.radius, capsuleGeo.halfHeight * 2.0f, debugColor);
#endif
            }
        }
    }
}
