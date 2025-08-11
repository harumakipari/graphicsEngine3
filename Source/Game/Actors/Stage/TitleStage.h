#ifndef TITLE_STAGE_H
#define TITLE_STAGE_H

#include "Core/Actor.h"
#include "Components/Render/MeshComponent.h" 
#include "Utils/EasingHandler.h"

class TitleStage :public Actor
{
public:
    //引数付きコンストラクタ
    TitleStage(std::string actorName) :Actor(actorName) {}
    virtual ~TitleStage() = default;

    std::shared_ptr<StaticMeshComponent> titleLogo; 
    std::shared_ptr<StaticMeshComponent> stage;
    std::shared_ptr<StaticMeshComponent> build;
    std::shared_ptr<StaticMeshComponent> trafficLight;
    //std::shared_ptr<SkeltalMeshComponent> trafficLight;
    void Initialize(const Transform& transform)override
    {
        std::shared_ptr<SceneComponent> parent = this->NewSceneComponent<SceneComponent>("empty");
        SetPosition(transform.GetLocation());
        SetQuaternionRotation(transform.GetRotation());
        SetScale(transform.GetScale());

        stage = this->NewSceneComponent<StaticMeshComponent>("stageComponent","empty");
        stage->SetModel("./Data/Models/Title/title_yuka_kabe.gltf");
        //stage->SetModel("./Data/Models/Stage/SpotLightStage/stydio_6.gltf");
        stage->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        stage->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });

        titleLogo = this->NewSceneComponent<StaticMeshComponent>("logoComponent", "empty");
        titleLogo->SetModel("./Data/Models/Title/title_rogo.gltf");
        titleLogo->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        titleLogo->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });        
        titleLogo->SetRelativeLocationDirect({ 0.0f,0.1f,-0.1f });      // y座標 1.9f で範囲外
        titleLogo->SetRelativeEulerRotationDirect({ 0.0f,-9.0f,0.0f });
        titleLogo->SetIsVisible(false);

        build = this->NewSceneComponent<StaticMeshComponent>("buildComponent", "empty");
        build->SetModel("./Data/Models/Title/title_bill.gltf");
        build->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        build->SetRelativeScaleDirect({ -1.0f,1.0f,-1.0f });

        //trafficLight = this->NewSceneComponent<SkeltalMeshComponent>("trafficLight", "empty");
        trafficLight = this->NewSceneComponent<StaticMeshComponent>("trafficLight", "empty");
        trafficLight->SetModel("./Data/Models/Stage/Props/traffic_light.gltf");
        trafficLight->model->modelCoordinateSystem = InterleavedGltfModel::CoordinateSystem::RH_Y_UP;
        trafficLight->model->emission = 1.0f;
        trafficLight->SetRelativeScaleDirect({ 2.0f,2.0f,2.0f });
        trafficLight->SetRelativeLocationDirect({ 4.25f,0.09f,7.27f });     
        trafficLight->SetRelativeEulerRotationDirect({ -12.4f,8.169f,53.431f });
        //CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelEmissionPS.cso", trafficLight->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
    }

    void SwitchPS(bool useDeffered)
    {
        if (useDeffered)
        {
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDefferedPS.cso", stage->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDefferedPS.cso", titleLogo->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDefferedPS.cso", build->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelDefferedPS.cso", trafficLight->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        }
        else
        {
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPS.cso", stage->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPS.cso", titleLogo->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPS.cso", build->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
            CreatePsFromCSO(Graphics::GetDevice(), "./Shader/GltfModelPS.cso", trafficLight->pipeLineState_.pixelShader.ReleaseAndGetAddressOf());
        }
    }

    void Update(float deltaTime)override
    {
        if (isPushButton&&!isSetEasing)
        {
            yEasing.Clear();
            yEasing.SetEasing(EaseType::OutQuart, 0.1f, 1.9f, 1.0f);
            isSetEasing = true;
            isPushButton = false;
        }

        if (OnPushBack && !isSetEasing)
        {
            yEasing.Clear();
            yEasing.SetEasing(EaseType::OutQuart, 1.9f, 0.1f, 1.0f);
            isSetEasing = true;
            OnPushBack = false;
        }

        yEasing.Update(easeY, deltaTime);
        titleLogo->SetRelativeLocationDirect({ 0.0f,easeY,-0.1f });      // y座標 1.9f で範囲外
    }

    void OnPushButton()
    {
        isPushButton = true;
        isSetEasing = false;
    }

    void OnPushBackToTitle()
    {
        OnPushBack = true;
        isSetEasing = false;
    }
private:
    EasingHandler yEasing;
    float easeY = 0.0f;
    bool isPushButton = false;

    bool isSetEasing =false;

    bool OnPushBack = false;
};

#endif // !TITLE_STAGE_H
