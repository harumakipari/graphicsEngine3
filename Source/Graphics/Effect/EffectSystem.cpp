#include "EffectSystem.h"

#ifdef USE_IMGUI
#define IMGUI_ENABLE_DOCKING
#include "../External/imgui/imgui.h"
#endif

#include "../../Core/ActorManager.h"
#include "../../Components/Effect/EffectComponent.h"
#include "Game/Actors/Player/Player.h"

void EffectSystem::Initialize()
{
    int numParticles[ARRAYSIZE(computeParticles)] = {
        50000,
        100000,
        100000,
        1000,
        1000,
        1000,
        10,
        10,
        10,
        10,
    };

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceViews[ARRAYSIZE(computeParticles)];
    const wchar_t* filePath[ARRAYSIZE(computeParticles)] = {
        L"./Data/Effect/Textures/particle.png",//beamCharge
        L"./Data/Effect/Textures/particle.png",//beam
        L"./Data/Effect/Textures/explosion.png",     // explosion
        L"./Data/Effect/Textures/particle.png",//紙吹雪
        L"./Data/Effect/Textures/particle.png",//spark
        L"./Data/Effect/Textures/particle.png",
        L"./Data/Effect/Textures/particle.png",
        L"./Data/Effect/Textures/particle.png",
        L"./Data/Effect/Textures/particle.png",
        L"./Data/Effect/Textures/particle.png",
    };
    DirectX::XMUINT2 splitCounts[ARRAYSIZE(computeParticles)] = {
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(5, 4),//explosion
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
        DirectX::XMUINT2(1, 1),
    };

    //コンピュートパーティクルシステム生成
    for (int i = 0; i < ARRAYSIZE(computeParticles); i++)
    {
        //テクスチャ読み込み
        LoadTextureFromFile(Graphics::GetDevice(), filePath[i], shaderResourceViews[i].ReleaseAndGetAddressOf(), NULL);

        //パーティクルシステム生成
        computeParticles[i] = std::make_unique<ComputeParticleSystem>(Graphics::GetDevice(),
            numParticles[i],
            shaderResourceViews[i],
            splitCounts[i]);
    }

    //仮
    model = std::make_unique<GltfModel>(Graphics::GetDevice(), "./Data/Models/Items/HeldEnergyCore/heldEnergyCore.gltf");
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmitParticlePS.cso", model->pixelShader.ReleaseAndGetAddressOf());
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmitParticlePS.cso", meshParticlePixelShader.ReleaseAndGetAddressOf());
    CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDissolvePS.cso", dissolvePixelShader.ReleaseAndGetAddressOf());

    //パーティクルシステム
        //particles = std::make_unique<decltype(particles)::element_type>(Graphics::GetDevice(), 10000);
        //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/LargeFlame01.tif", particles->particleTexture.GetAddressOf(), NULL);
        //particles->particleSystemData.spriteSheetGrid = { 8,8 };
        //LoadTextureFromFile(device, L"./Data/Effect/Particles/Ramp02.png", colorTemperChart.GetAddressOf(), NULL); // blue
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/ramp01.png", colorTemperChart.GetAddressOf(), NULL); // red 
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Textures/target.png", projectionTexture.GetAddressOf(), NULL);
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/noise1.png", maskTexture.GetAddressOf(), NULL); //noiseTexture

    //パーティクルプリセットパラメータセット
    //EmitterData charge;
    //charge.count = 50;
    //charge.blendMode = 1;
    //charge.data.type = 1;
    //charge.data.emissionOffset.x = -10.f;
    //charge.data.emissionOffset.y = 10.f;
    //charge.data.emissionConeAngle.y = 3.1416f;
    //charge.data.emissionSpeed.y = 3.f;
    //charge.data.strength = 0.67f;
    ////charge->particleSystemData.strength = 1.f;
    ////charge.data.emissionSize.x = 0.143f;
    ////charge.data.emissionSize.y = 0.05f;
    //charge.data.emissionSize.x = 0.3f;
    //charge.data.emissionSize.y = 1.f;
    //charge.data.emissionStartColor = { 0,216.f / 255.f, 1.f,1.f };
    //charge.data.emissionEndColor = { 0,216.f / 255.f, 1.f,1.f };
    //charge.data.spawnDelay.x = 0.0f;
    //charge.data.spawnDelay.y = 0.3f;
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/RoundSoftParticle.tif", charge.texture.ReleaseAndGetAddressOf(), NULL);
    //presets.push_back(charge);

#if 0
    ParticleSystem* charge = CreateEmitter(50);
    charge->blendMode = 1;
    charge->particleSystemData.type = 1;
    charge->particleSystemData.emissionOffset.x = -10.f;
    charge->particleSystemData.emissionOffset.y = 10.f;
    charge->particleSystemData.emissionConeAngle.y = 3.1416f;
    charge->particleSystemData.emissionSpeed.y = 3.f;
    charge->particleSystemData.strength = 0.67f;
    //charge->particleSystemData.strength = 1.f;
    charge->particleSystemData.emissionSize.x = 0.143f;
    charge->particleSystemData.emissionSize.y = 0.05f;
    charge->particleSystemData.emissionSize.x = 0.3f;
    charge->particleSystemData.emissionSize.y = 1.f;
    charge->particleSystemData.emissionStartColor = { 0,216.f / 255.f, 1.f,1.f };
    charge->particleSystemData.emissionEndColor = { 0,216.f / 255.f, 1.f,1.f };
    charge->particleSystemData.spawnDelay.x = 0.0f;
    charge->particleSystemData.spawnDelay.y = 0.3f;
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/RoundSoftParticle.tif", charge->particleTexture.ReleaseAndGetAddressOf(), NULL);
#endif // 0

    //EmitterData beam;
    //beam.count = 20;
    //beam.blendMode = 1;
    //beam.data.direction = { 0,0,0 };
    //beam.data.spawnDelay = { 0.3f,0.73f };
    //beam.data.lifespan = { 2.2f,4.6f };
    //beam.data.spawnDelay = { 0.06f,0.12f };
    //beam.data.lifespan = { 0.5f,0.75f };
    //beam.data.gravity = -0.00f;
    //beam.data.strength = 1.f;
    //beam.data.emissionOffset = { -0.15f, 0.15f };
    //beam.data.emissionStartColor = { 0,1,1,1 };
    //beam.data.emissionEndColor = { 0,1,1,1 };
    //beam.data.loop = true;
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/Particle01.png", beam.texture.ReleaseAndGetAddressOf(), NULL);
    //presets.push_back(beam);
#if 0
    ParticleSystem* beam = CreateEmitter(20);
    beam->particleSystemData.direction = { 0,0,0 };
    beam->particleSystemData.spawnDelay = { 0.3f,0.73f };
    beam->particleSystemData.lifespan = { 2.2f,4.6f };

    beam->particleSystemData.spawnDelay = { 0.06f,0.12f };
    beam->particleSystemData.lifespan = { 0.5f,0.75f };

    beam->particleSystemData.gravity = -0.00f;
    beam->particleSystemData.strength = 1.f;
    beam->particleSystemData.emissionOffset = { -0.15f, 0.15f };
    beam->particleSystemData.emissionStartColor = { 0,1,1,1 };
    beam->particleSystemData.emissionEndColor = { 0,1,1,1 };

    beam->particleSystemData.loop = true;
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/Particle01.png", beam->particleTexture.ReleaseAndGetAddressOf(), NULL);
#endif // 0

    //EmitterData explosion;
    //explosion.count = 1000;
    //explosion.blendMode = 0;
    //explosion.data.emissionOffset.x = -1.5f;
    //explosion.data.emissionOffset.y = 1.5f;
    //explosion.data.emissionConeAngle.y = 0.2f;
    //explosion.data.emissionAngularSpeed.y = 2.25f;
    //explosion.data.spawnDelay.x = 0.f;
    //explosion.data.spawnDelay.y = 0.5f;
    //explosion.data.strength = 2.75f;
    //explosion.data.emissionSize.x = 1.f;
    //explosion.data.emissionSize.y = 1.5f;
    //explosion.data.spriteSheetGrid = { 6,6 };
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/FireExplosion.tif", explosion.texture.ReleaseAndGetAddressOf(), NULL);
    //presets.push_back(explosion);

#if 0
    ParticleSystem* explosion = CreateEmitter(1000);
    explosion->blendMode = 0;
    explosion->particleSystemData.emissionOffset.x = -1.5f;
    explosion->particleSystemData.emissionOffset.y = 1.5f;
    explosion->particleSystemData.emissionConeAngle.y = 0.2f;
    explosion->particleSystemData.emissionAngularSpeed.y = 2.25f;
    explosion->particleSystemData.spawnDelay.x = 0.f;
    explosion->particleSystemData.spawnDelay.y = 0.5f;
    explosion->particleSystemData.strength = 2.75f;
    explosion->particleSystemData.emissionSize.x = 1.f;
    explosion->particleSystemData.emissionSize.y = 1.5f;
    explosion->particleSystemData.spriteSheetGrid = { 6,6 };
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/FireExplosion.tif", explosion->particleTexture.ReleaseAndGetAddressOf(), NULL);
#endif // 0

    //EmitterData shockwave;
    //shockwave.count = 3;
    //shockwave.data.emissionConeAngle = {};
    //shockwave.data.emissionAngularSpeed = {};
    //shockwave.data.emissionSpeed = { 1,1 };
    //shockwave.data.spawnDelay = {};
    //shockwave.data.direction = { 0,1,0 };
    //shockwave.data.gravity = -0.5f;
    //shockwave.data.strength = 10.f;
    //shockwave.data.emissionSize = { 0,5 };
    //shockwave.data.fadeDuration = { 0.f,0.2f };
    //shockwave.data.lifespan = { 0.25f,0.25f };
    //shockwave.data.emissionStartColor = { 1,0.2f,0.2f, 1 };
    //shockwave.data.emissionEndColor = { 1,0.2f,0.2f, 1 };
    //shockwave.data.isStatic = true;
    //LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/Shock_wave002.png", shockwave.texture.ReleaseAndGetAddressOf(), NULL);
    //presets.push_back(shockwave);

#if 0
    ParticleSystem* shockwave = CreateEmitter(3);
    shockwave->particleSystemData.emissionConeAngle = {};
    shockwave->particleSystemData.emissionAngularSpeed = {};
    shockwave->particleSystemData.emissionSpeed = { 1,1 };
    shockwave->particleSystemData.spawnDelay = {};
    shockwave->particleSystemData.direction = { 0,1,0 };
    shockwave->particleSystemData.gravity = -0.5f;
    shockwave->particleSystemData.strength = 10.f;
    shockwave->particleSystemData.emissionSize = { 0,5 };
    shockwave->particleSystemData.fadeDuration = { 0.f,0.2f };
    shockwave->particleSystemData.lifespan = { 0.25f,0.25f };
    shockwave->particleSystemData.emissionStartColor = { 1,0.2f,0.2f, 1 };
    shockwave->particleSystemData.emissionEndColor = { 1,0.2f,0.2f, 1 };
    shockwave->particleSystemData.isStatic = true;
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/Shock_wave002.png", shockwave->particleTexture.ReleaseAndGetAddressOf(), NULL);
#endif // 0

    //EmitterData spark;
    //spark.count = 1000;
    //spark.data.emissionConeAngle.y = 1.4f;
    //spark.data.emissionSpeed = { 1.8f, 3.2f };
    //spark.data.strength = 2.5f;
    //spark.data.gravity = -9.f;
    //spark.data.emissionSize.x = 0.1f;
    //spark.data.emissionSize.y = 0.05f;
    //spark.data.spawnDelay.x = 0.0f;
    //spark.data.spawnDelay.y = 0.123f;
    //MakeDummyTexture(Graphics::GetDevice(), spark.texture.ReleaseAndGetAddressOf(), 0xFFFFFFFF, 16);
    //presets.push_back(spark);

#if 0
    ParticleSystem* spark = CreateEmitter(1000);
    spark->particleSystemData.emissionConeAngle.y = 1.4f;
    spark->particleSystemData.emissionSpeed = { 1.8f, 3.2f };
    spark->particleSystemData.strength = 2.5f;
    spark->particleSystemData.gravity = -9.f;
    spark->particleSystemData.emissionSize.x = 0.1f;
    spark->particleSystemData.emissionSize.y = 0.05f;
    spark->particleSystemData.spawnDelay.x = 0.0f;
    spark->particleSystemData.spawnDelay.y = 0.123f;
#endif // 0

#if 0
    ParticleSystem* missileFire = CreateEmitter(20000);
    missileFire->particleSystemData.direction = { 0,0,0 };
    missileFire->particleSystemData.spawnDelay = { 0.3f,0.73f };
    missileFire->particleSystemData.lifespan = { 2.2f,4.6f };
    missileFire->particleSystemData.gravity = -0.001f;
    missileFire->particleSystemData.strength = 1.f;
    missileFire->particleSystemData.emissionOffset = { -0.15f, 0.15f };
    missileFire->particleSystemData.emissionStartColor = { 0,1,1,1 };
    missileFire->particleSystemData.emissionEndColor = { 0,1,1,1 };
    missileFire->particleSystemData.spriteSheetGrid = { 7,7 };
    missileFire->particleSystemData.loop = true;
    LoadTextureFromFile(Graphics::GetDevice(), L"./Data/Effect/Particles/Fireball.tif", missileFire->particleTexture.ReleaseAndGetAddressOf(), NULL);
#endif // 0



    //パラメータ保存
    for (auto& particle : particles) {
        particle->presetData = particle->particleSystemData;
    }
}

void EffectSystem::Update(float deltaTime)
{
    Scene* currentScene = Scene::GetCurrentScene();  // 現在のシーン取得
    if (!currentScene) return;
    auto actorManager = currentScene->GetActorManager();  // ActorManager取得

    for (auto& actor : actorManager->allActors_) {
        if (!actor->rootComponent_ || !actor->isActive) {
            continue;
        }

        std::vector<EffectComponent*> effectComponents;
        actor->GetComponents<EffectComponent>(effectComponents);

        for (EffectComponent* effectComponent : effectComponents)
        {
            if (effectComponent->IsPlay())
            {
                size_t index = static_cast<size_t>(effectComponent->GetEffectType());
                if (ARRAYSIZE(computeParticles) > index)
                {
                    //ParticleSystem* particle = particles[index].get();

                    //DirectX::XMFLOAT3 pos = effectComponent->GetComponentLocation();
                    //particle->particleSystemData = particle->presetData;
                    //particle->particleSystemData.emissionPosition = { pos.x, pos.y, pos.z, 1.f };

                    //particle->Initialize(Graphics::GetDeviceContext(), 0);
                    SpawnEmitter(effectComponent);

                    //ビームのループ以外
                    if (index > 1)
                    {
                        effectComponent->Initialized();
                    }
                }
            }
        }
    }
#if 1
#if 0
    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        ParticleSystem* particle = particles[0].get();

        DirectX::XMFLOAT3 pos = ActorManager::GetActorByName("actor")->GetPosition();
        particle->particleSystemData = particle->presetData;
        particle->particleSystemData.emissionPosition = { pos.x, pos.y, pos.z, 1.f };

        particle->Initialize(Graphics::GetDeviceContext(), 0);
    }
#endif // 0


    //パーティクル更新処理
    if (integrateParticles)
    {
        //Graphics::GetDeviceContext()->CSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());

#if 0
        static bool runOnce = false;
        if (!runOnce && (GetAsyncKeyState('Z') & 0x8000)) {
            particles[3]->Initialize(Graphics::GetDeviceContext(), 0);
            runOnce = true;
        }
        if (runOnce)
        {
            DirectX::XMFLOAT3 pos = ActorManager::GetActorByName("actor")->GetPosition();
            particles[3]->particleSystemData.loop = true;
            particles[3]->particleSystemData.emissionPosition = { pos.x, pos.y, pos.z, 1.f };
            particles[3]->Integrate(Graphics::GetDeviceContext(), deltaTime);
        }
#endif // 0


#if 1
        for (auto& effect : particles)
        {
            effect->Integrate(Graphics::GetDeviceContext(), deltaTime);
        }
#else
        for (auto& actor : ActorManager::allActors_) {
            if (!actor->rootComponent_ || !actor->isActive) {
                continue;
            }

            std::vector<EffectComponent*> effectComponents;
            actor->GetComponents<EffectComponent>(effectComponents);

            for (EffectComponent* effectComponent : effectComponents)
            {
                size_t index = static_cast<size_t>(effectComponent->GetEffectType());
                if (particles.size() > index)
                {
                    ParticleSystem* emitter = particles[index].get();

                    //状態に応じた処理
                    switch (effectComponent->GetEffectState())
                    {
                    case EffectComponent::EffectState::Active:
                        DirectX::XMFLOAT3 pos = effectComponent->GetComponentLocation();
                        emitter->particleSystemData.emissionPosition = { pos.x, pos.y, pos.z, 1.f };

                        emitter->Integrate(Graphics::GetDeviceContext(), deltaTime);
                        break;
                    case EffectComponent::EffectState::Finished:
                        emitter->particleSystemData.loop = false;
                        emitter->Integrate(Graphics::GetDeviceContext(), deltaTime);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
#endif // 0
        for (auto& effect : particles)
        {
            if (effect->particleSystemData.state == 2 || allParticleIntegrate)
                effect->Integrate(Graphics::GetDeviceContext(), deltaTime);
        }

    }
#endif // 0

#if 0
    if (GetAsyncKeyState(VK_RETURN) & 0x8000)
    {
        DirectX::XMFLOAT3 pos = ActorManager::GetActorByName("actor")->GetPosition();
        int max = 50;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            data.parameter.x = 1;
            data.parameter.y = 1.5f;
            data.parameter.w = (rand() % 5 + 1) * 0.1f;
            //	発生位置
            float distance = (rand() % 40 + 10) * 0.1f;
            float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.1415926f;
            float phi = static_cast<float>(rand()) / RAND_MAX * 3.1415926f;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            data.position.x = pos.x + (x * distance);
            data.position.y = pos.y + (y * distance);
            data.position.z = pos.z + (z * distance);
            //	初速力
            data.velocity.x = (rand() % 32 * 0.2f) + distance;
            data.velocity.y = (rand() % 32 * 0.2f) + distance;
            data.velocity.z = (rand() % 32 * 0.2f) + distance;
            //	加速力
            data.acceleration.x = (rand() % 20 - 10) * 0.1f;
            data.acceleration.y = (rand() % 20 - 10) * 0.1f;
            data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            //	大きさ
            data.scale.x = (rand() % 20 + 20) * 0.01f;
            data.scale.y = (rand() % 20 + 20) * 0.01f;
            data.scale.z = 1.0f;
            data.scale.w = 1.0f;
            //色
            data.color = { 0,1,1,1 };
            //	ターゲット位置
            data.customData.x = pos.x;
            data.customData.y = pos.y + 1.0f;
            data.customData.z = pos.z;
            data.customData.w = 1.0f;

            computeParticles[0]->Emit(data);
        }
    }
#endif // 0


#if 0
    //	降雪
    if (::GetAsyncKeyState('X') & 0x8000)
    {
        DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3((rand() % 30 - 15) * 0.1f, rand() % 30 * 0.1f + 1, (rand() % 30 - 15) * 0.1f + 3);
        int max = 100;
        for (int i = 0; i < max; i++)
        {
            //	発生位置
            DirectX::XMFLOAT3 p = { 0,0,0 };
            p.x = pos.x + (rand() % 10001 - 5000) * 0.01f;
            p.y = pos.y;
            p.z = pos.z + (rand() % 10001 - 5000) * 0.01f;
            //	発生方向
            DirectX::XMFLOAT3 v = { 0,0,0 };
            v.y = -1.2f;
            //	力
            DirectX::XMFLOAT3 f = { 0,0,0 };
            f.x = (rand() % 10001) * 0.00001f + 0.1f;
            f.z = (rand() % 10001 - 5000) * 0.00001f;
            //	大きさ
            DirectX::XMFLOAT2 s = { .5f,.5f };

            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            data.parameter.x = 12;
            data.parameter.y = 5.0f;
            //	発生位置
            data.position.x = p.x;
            data.position.y = p.y;
            data.position.z = p.z;
            //	発生方向
            data.velocity.x = v.x;
            data.velocity.y = v.y;
            data.velocity.z = v.z;
            //	加速力
            data.acceleration.x = f.x;
            data.acceleration.y = f.y;
            data.acceleration.z = f.z;
            //	大きさ
            data.scale.x = s.x;
            data.scale.y = s.y;
            data.scale.z = 0.0f;

            computeParticles[7]->Emit(data);
        }
    }
    if (GetAsyncKeyState('Z') & 0x8000)
    {
        DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3((float)(rand() % 30 - 15), 0, (float)(rand() % 30 - 15));
        int max = 100;
        for (int i = 0; i < max; i++)
        {
            DirectX::XMFLOAT3 p;
            p.x = pos.x;
            p.y = pos.y;
            p.z = pos.z;

            DirectX::XMFLOAT3 v = { 0,0,0 };
            v.x = (rand() % 10001 - 5000) * 0.001f;
            v.y = (rand() % 10001) * 0.0002f + 1.2f;
            v.z = (rand() % 10001 - 5000) * 0.001f;

            DirectX::XMFLOAT3 f = { 0,-0.2f,0 };
            DirectX::XMFLOAT2 s = { 0.5f,0.5f };

            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            data.parameter.x = 2;
            data.parameter.y = 1.0f;
            //	発生位置
            data.position.x = p.x;
            data.position.y = p.y;
            data.position.z = p.z;
            //	発生方向
            data.velocity.x = v.x;
            data.velocity.y = v.y;
            data.velocity.z = v.z;
            //	加速力
            data.acceleration.x = f.x;
            data.acceleration.y = f.y;
            data.acceleration.z = f.z;
            //	大きさ
            data.scale.x = s.x;
            data.scale.y = s.y;
            data.scale.z = 0.0f;
            data.scale.w = 0.0f;

            computeParticles[8]->Emit(data);
        }
    }

    //	モデルの描画位置からパーティクルを生成
    if (::GetAsyncKeyState('V') & 0x8000)
    {
        //	ピクセル側でのエミット処理開始
        computeParticles[9]->PixelEmitBegin(Graphics::GetDeviceContext(), deltaTime);

        RenderState::BindDepthStencilState(Graphics::GetDeviceContext(), DEPTH_STATE::ZT_OFF_ZW_OFF, 0);
        RenderState::BindRasterizerState(Graphics::GetDeviceContext(), RASTER_STATE::SOLID_CULL_NONE);

        //	モデルの姿勢
        float scale = 1.0f;
        float rotation = 0.0f;
        DirectX::XMFLOAT3 translation = { 0.0f, 0.0f, 0.0f };

        //	モデル適当に描画
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX Scale = DirectX::XMMatrixScaling(scale, scale, scale);
        DirectX::XMMATRIX Rotation = DirectX::XMMatrixRotationRollPitchYaw(rotation, rotation, rotation);
        DirectX::XMFLOAT4X4 world_matrix;

        World = Scale * Rotation * DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
        DirectX::XMStoreFloat4x4(&world_matrix, World);
        model->Render(Graphics::GetDeviceContext(), world_matrix, RenderPass::Opaque);

        //World = Scale * Rotation * DirectX::XMMatrixTranslation(translation.x + 1, translation.y, translation.z);
        //DirectX::XMStoreFloat4x4(&world_matrix, World);
        //model->Render(Graphics::GetDeviceContext(), world_matrix, RenderPass::Blend);

        //	ピクセル側でのエミット処理終了
        computeParticles[9]->PixelEmitEnd(Graphics::GetDeviceContext());
    }
#endif // 0


    //コンピュートパーティクル更新
    for (auto& computeParticle : computeParticles)
    {
        computeParticle->Update(Graphics::GetDeviceContext(), deltaTime);
    }
}

void EffectSystem::SpawnEmitter(EffectComponent* effectComponent)
{
    DirectX::XMFLOAT3 pos = effectComponent->GetActor()->GetPosition();
    switch (effectComponent->GetEffectType())
    {
    case EffectComponent::EffectType::BeamCharge:
    {
        if (std::shared_ptr<Player> player = std::dynamic_pointer_cast<Player>(effectComponent->GetActor()))
        {
            DirectX::XMFLOAT3 forward = player->GetForward();
            DirectX::XMVECTOR Forward = DirectX::XMLoadFloat3(&forward);
            DirectX::XMVECTOR Pos = DirectX::XMLoadFloat3(&pos);
            DirectX::XMStoreFloat3(&pos, DirectX::XMVectorAdd(Pos, DirectX::XMVectorScale(Forward, 1.25f)));
        }

        int max = 1;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            int random = rand() % 2;
            data.parameter.x = static_cast<float>(random);
            data.parameter.y = 1.0f;
            data.parameter.w = (rand() % 10) * 0.01f;
            //	発生位置
            float distance = (rand() % 40 + 10) * 0.1f;
            float theta = static_cast<float>(rand()) / RAND_MAX * 2.0f * 3.1415926f;
            float phi = static_cast<float>(rand()) / RAND_MAX * 3.1415926f;
            float x = sin(phi) * cos(theta);
            float y = cos(phi);
            float z = sin(phi) * sin(theta);

            if (random == 0)
            {
                data.position.x = pos.x;
                data.position.y = pos.y;
                data.position.z = pos.z;
            }
            else
            {
                data.position.x = pos.x + (x * distance);
                data.position.y = pos.y + (y * distance);
                data.position.z = pos.z + (z * distance);
                //	初速力
                data.velocity.x = (rand() % 32 * 0.2f) + distance;
                data.velocity.y = (rand() % 32 * 0.2f) + distance;
                data.velocity.z = (rand() % 32 * 0.2f) + distance;
                //	加速力
                data.acceleration.x = (rand() % 20 - 10) * 0.1f;
                data.acceleration.y = (rand() % 20 - 10) * 0.1f;
                data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            }
            //	大きさ
            data.scale.x = (rand() % 10 + 10) * 0.01f;
            data.scale.y = (rand() % 10 + 10) * 0.01f;
            data.scale.z = 0.5f;
            data.scale.w = 0.5f;
            //色
            data.color = { 0,1,1,1 };
            //	ターゲット位置
            data.customData.x = pos.x;
            data.customData.y = pos.y;
            data.customData.z = pos.z;
            data.customData.w = 1.0f;

            computeParticles[0]->Emit(data);
        }
        break;
    }
    case EffectComponent::EffectType::Beam:
    {
        //ビームの強さ
        float maxPower = effectComponent->GetEffectMaxPower();
        float power = effectComponent->GetEffectPower();
        int max = 5;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            data.parameter.x = static_cast<float>(rand() % 2);
            data.parameter.y = rand() % 50 * 0.01f + 0.5f;
            data.parameter.w = rand() % 10 * 0.01f;
            //	発生位置
            data.position.x = pos.x + rand() % 10 * 0.01f;
            data.position.y = pos.y + rand() % 10 * 0.01f;
            data.position.z = pos.z + rand() % 10 * 0.01f;
            //	初速力
            data.velocity.x = data.parameter.x == 1 ? 1.0f : 0.0f;
            data.velocity.y = data.parameter.x == 1 ? 1.0f : 0.0f;
            data.velocity.z = data.parameter.x == 1 ? 1.0f : 0.0f;
            //	加速力
            //data.acceleration.x = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.y = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            //	大きさ
            //data.scale.x = (rand() % 50 * 0.01f) + 2.5f;
            //data.scale.y = (rand() % 50 * 0.01f) + 2.5f;
            data.scale.x = (rand() % 50 * 0.01f) + 1.f + (power / maxPower) * 3.0f;
            data.scale.y = (rand() % 50 * 0.01f) + 1.f + (power / maxPower) * 3.0f;
            data.scale.z = 0.3f;
            data.scale.w = 0.3f;
            //色
            data.color = { 0,1,1,1 };
            //	ターゲット位置
            data.customData.x = pos.x;
            data.customData.y = pos.y;
            data.customData.z = pos.z;
            data.customData.w = 1.0f;

            computeParticles[1]->Emit(data);
        }
        break;
    }
    case EffectComponent::EffectType::Explosion:
    {
        int max = 50;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            data.parameter.x = 0;
            data.parameter.y = 1.25f;
            data.parameter.w = /*(rand() % 5)*/i * 0.01f;
            //	発生位置
            data.position.x = pos.x + (rand() % 300 - 150) * 0.01f;
            data.position.y = pos.y + (rand() % 300 - 150) * 0.01f;
            data.position.z = pos.z + (rand() % 300 - 150) * 0.01f;
            //	初速力
            data.velocity.x = ((rand() % 32 - 16) * 0.1f);
            data.velocity.y = (rand() % 32 * 0.1f);
            data.velocity.z = ((rand() % 32 - 16) * 0.1f);
            //	加速力
            data.acceleration.x = (rand() % 20 - 10) * 0.1f;
            data.acceleration.y = 1.f;
            data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            //	大きさ
            data.scale.x = (rand() % 50 + 100) * 0.01f;
            data.scale.y = (rand() % 50 + 100) * 0.01f;
            data.scale.z = 3.f;
            data.scale.w = 3.f;
            //色
            data.color = { 1,1,1,1 };
            //	ターゲット位置
            data.customData.x = pos.x;
            data.customData.y = pos.y;
            data.customData.z = pos.z;
            data.customData.w = 1.0f;
            computeParticles[2]->blendState = BLEND_STATE::ALPHA;
            computeParticles[2]->Emit(data);
        }
        break;
    }
    case EffectComponent::EffectType::ShockWave:
    {

        break;
    }
    case EffectComponent::EffectType::Spark:
    {
        int max = 100;
        for (int i = 0; i < max; i++)
        {
            DirectX::XMFLOAT3 p;
            p.x = pos.x;
            p.y = pos.y;
            p.z = pos.z;

            DirectX::XMFLOAT3 v = { 0,0,0 };
            v.x = (rand() % 10001 - 5000) * 0.001f;
            v.y = (rand() % 10001) * 0.0002f + 1.2f;
            v.z = (rand() % 10001 - 5000) * 0.001f;

            DirectX::XMFLOAT3 f = { 0,-0.2f,0 };
            DirectX::XMFLOAT2 s = { 0.3f,0.3f };

            ComputeParticleSystem::EmitParticleData data;
            //	更新タイプ
            data.parameter.x = 2;
            data.parameter.y = 1.0f;
            //	発生位置
            data.position.x = p.x;
            data.position.y = p.y;
            data.position.z = p.z;
            //	発生方向
            data.velocity.x = v.x;
            data.velocity.y = v.y;
            data.velocity.z = v.z;
            //	加速力
            data.acceleration.x = f.x;
            data.acceleration.y = f.y;
            data.acceleration.z = f.z;
            //色
            data.color = { 1,0.5f,0,1 };
            //	大きさ
            data.scale.x = s.x;
            data.scale.y = s.y;
            data.scale.z = 0.0f;
            data.scale.w = 0.0f;

            computeParticles[4]->Emit(data);
        }
        break;
    }
    default:
        break;
    }
}

void EffectSystem::SpawnEmitter(int type, DirectX::XMFLOAT3 pos, float power)
{
    switch (type)
    {
    case 0:
    {
        //ビームの強さ
        int max = 2;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            data.parameter.x = static_cast<float>(rand() % 2);
            data.parameter.y = rand() % 50 * 0.01f + 0.5f;
            data.parameter.w = rand() % 10 * 0.01f;
            //	発生位置
            data.position.x = pos.x;
            data.position.y = pos.y;
            data.position.z = pos.z;
            //	初速力
            data.velocity.x = data.parameter.x == 1 ? 1.0f : 0.0f;
            data.velocity.y = data.parameter.x == 1 ? 1.0f : 0.0f;
            data.velocity.z = data.parameter.x == 1 ? 1.0f : 0.0f;
            //	加速力
            //data.acceleration.x = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.y = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            //	大きさ
            //data.scale.x = (rand() % 50 * 0.01f) + 2.5f;
            //data.scale.y = (rand() % 50 * 0.01f) + 2.5f;
            data.scale.x = power;
            data.scale.y = power;
            data.scale.z = 0.3f;
            data.scale.w = 0.3f;
            //色
            data.color = { 1,0,0,1 };
            //	ターゲット位置
            data.customData.x = pos.x;
            data.customData.y = pos.y;
            data.customData.z = pos.z;
            data.customData.w = 1.0f;

            computeParticles[1]->Emit(data);
        }
        break;
    }
    case 1:
    {
        //ビームの強さ
        int max = 7200;
        float speed = 50.0f;
        for (int i = 0; i < max; i++)
        {
            ComputeParticleSystem::EmitParticleData data;
            data.parameter.x = 0;
            data.parameter.y = 0.5f;
            //	発生位置
            data.position.x = pos.x;
            data.position.y = pos.y;
            data.position.z = pos.z;
            //	初速力
            data.velocity.x = sin(((float)i / max) * XM_2PI) * speed;
            data.velocity.y = 0.0f;
            data.velocity.z = cos(((float)i / max) * XM_2PI) * speed;
            //	加速力
            //data.acceleration.x = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.y = (rand() % 20 - 10) * 0.1f;
            //data.acceleration.z = (rand() % 20 - 10) * 0.1f;
            //	大きさ
            //data.scale.x = (rand() % 50 * 0.01f) + 2.5f;
            //data.scale.y = (rand() % 50 * 0.01f) + 2.5f;
            data.scale.x = 0.5f;
            data.scale.y = 0.5f;
            data.scale.z = 0.5f;
            data.scale.w = 0.5f;
            //色
            data.color = { 1,0,0,1 };

            computeParticles[1]->Emit(data);
        }
        break;
    }
    default:
        break;
    }
}

void EffectSystem::ResultParticle(DirectX::XMFLOAT4 color)
{
#if 1
    DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3((float)(rand() % 30 - 15), 0.5f, (float)(rand() % 30 - 15));
    float delay = rand() % 300 * 0.01f;
    int max = 100;
    for (int i = 0; i < max; i++)
    {
        DirectX::XMFLOAT3 p;
        p.x = pos.x;
        p.y = pos.y;
        p.z = pos.z;

        DirectX::XMFLOAT3 v = { 0,0,0 };
        v.x = (rand() % 10001 - 5000) * 0.001f;
        v.y = (rand() % 10001) * 0.0002f + 1.2f;
        v.z = (rand() % 10001 - 5000) * 0.001f;

        DirectX::XMFLOAT3 f = { 0,-0.2f,0 };
        DirectX::XMFLOAT2 s = { 0.5f,0.5f };

        ComputeParticleSystem::EmitParticleData data;
        //	更新タイプ
        data.parameter.x = 2;
        data.parameter.y = 1.0f;
        data.parameter.w = delay;
        //	発生位置
        data.position.x = p.x;
        data.position.y = p.y;
        data.position.z = p.z;
        //	発生方向
        data.velocity.x = v.x;
        data.velocity.y = v.y;
        data.velocity.z = v.z;
        //	加速力
        data.acceleration.x = f.x;
        data.acceleration.y = f.y;
        data.acceleration.z = f.z;
        //	大きさ
        data.scale.x = s.x;
        data.scale.y = s.y;
        data.scale.z = 0.0f;
        data.scale.w = 0.0f;

        data.color = color;

        computeParticles[3]->Emit(data);
    }
#else
    DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3((rand() % 30 - 15) * 0.1f, rand() % 10 * 0.1f + 2, (rand() % 30 - 15) * 0.1f);
    int max = 1;
    for (int i = 0; i < max; i++)
    {
        //	発生位置
        DirectX::XMFLOAT3 p = { 0,0,0 };
        p.x = pos.x + (rand() % 1001 - 500) * 0.01f;
        p.y = pos.y;
        p.z = pos.z + (rand() % 1001 - 500) * 0.01f;
        //	発生方向
        DirectX::XMFLOAT3 v = { 0,0,0 };
        v.y = -1.2f;
        //	力
        DirectX::XMFLOAT3 f = { 0,0,0 };
        f.x = (rand() % 10001) * 0.00001f + 0.1f;
        f.z = (rand() % 10001 - 5000) * 0.00001f;
        //	大きさ
        DirectX::XMFLOAT2 s = { .5f,.5f };

        ComputeParticleSystem::EmitParticleData data;
        //	更新タイプ
        data.parameter.x = 12;
        data.parameter.y = 5.0f;
        //	発生位置
        data.position.x = p.x;
        data.position.y = p.y;
        data.position.z = p.z;
        //	発生方向
        data.velocity.x = v.x;
        data.velocity.y = v.y;
        data.velocity.z = v.z;
        //	加速力
        data.acceleration.x = f.x;
        data.acceleration.y = f.y;
        data.acceleration.z = f.z;
        //	大きさ
        data.scale.x = s.x;
        data.scale.y = s.y;
        data.scale.z = 0.0f;

        computeParticles[3]->Emit(data);
    }
#endif // 0

}

void EffectSystem::Render(ID3D11DeviceContext* immediateContext)
{
    // PARTICLE
    RenderState::BindBlendState(immediateContext, BLEND_STATE::ADD);
    RenderState::BindDepthStencilState(immediateContext, DEPTH_STATE::ZT_ON_ZW_OFF);
    RenderState::BindRasterizerState(immediateContext, RASTERRIZER_STATE::SOLID_CULL_NONE);

#if 0
    for (auto& effect : particles)
    {
        immediateContext->PSSetShaderResources(0, 1, effect->particleTexture.GetAddressOf());
        immediateContext->GSSetShaderResources(0, 1, colorTemperChart.GetAddressOf());
        effect->Render(immediateContext);
    }
#endif // 0


    //コンピュートパーティクル描画処理
    for (auto& computeParticle : computeParticles)
    {
        RenderState::BindBlendState(immediateContext, computeParticle->blendState);
        computeParticle->Render(immediateContext);
    }
}

void EffectSystem::DrawGUI()
{
#ifdef USE_IMGUI

    for (auto& computeParticle : computeParticles)
    {
        computeParticle->DrawGUI();
    }



    ImGui::Checkbox("allParticleIntegrate", &allParticleIntegrate);
    ImGui::Checkbox("integrate_particles", &integrateParticles);

    ImGui::Separator();

    static int count = 10000;
    ImGui::InputInt("particleCount", &count);

    if (ImGui::Button("+", ImVec2(30, 30)) && particles.size() < particles.max_size())
    {
        ParticleSystem* emitter = CreateEmitter(count);
        emitter->particleSystemData.state = 2;
    }
    ImGui::SameLine();
    if (ImGui::Button("-", ImVec2(30, 30)) && particles.size() > 0)
    {
        particles.pop_back();
    }


    ImGui::Separator();
    //それぞれのエフェクトのデバッグGUI描画
    int i = 0;
    for (auto& effect : particles)
    {
        if (ImGui::TreeNode(("effect" + std::to_string(i++)).c_str()))
        {
            ImGui::Separator();
            effect->DrawGUI();

            ImGui::TreePop();
        }
        ImGui::Separator();
    }
#endif
}