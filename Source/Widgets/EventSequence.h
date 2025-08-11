#pragma once
#include "UIComponent.h"
#include <vector>
#include <memory>
#include <functional>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI


//�X�e�b�v���N���X
class IStep {
public:
    virtual ~IStep() {}
    virtual void Start(GameObject* owner) = 0;
    virtual void Update(float elapsedTime) = 0;
    virtual bool IsFinished() const = 0;
    virtual void Reset() {};
};

class WaitStep;
class CallStep;
class ConditionalStep;

//�C�x���g�V�[�P���X
class EventSequence : public UIComponent {
    std::vector<std::unique_ptr<IStep>> steps;
    size_t currentIndex = 0;
    bool playing = false;

public:
    void AddStep(std::unique_ptr<IStep> step) {
        steps.emplace_back(std::move(step));
    }

    void AddStepFront(std::unique_ptr<IStep> step) {
        steps.insert(steps.begin(), std::move(step));
    }

    void StartSequence() {
        if (steps.empty()) return;
        playing = true;
        currentIndex = 0;
        for (auto& step : steps) step->Reset();
        steps[currentIndex]->Start(this->gameObject);
    }

    virtual void Update(float elapsedTime) override {
        if (!playing || currentIndex >= steps.size()) return;

        auto& step = steps[currentIndex];
        step->Update(elapsedTime);

        if (step->IsFinished()) {
            ++currentIndex;
            if (currentIndex < steps.size()) {
                steps[currentIndex]->Start(this->gameObject);
            }
            else {
                playing = false;
            }
        }
    }

    bool IsPlaying() const { return playing; }

    void DrawProperty() override {
#ifdef USE_IMGUI
        bool playing = this->playing;
        ImGui::Checkbox("playing", &playing);
        ImGui::Text("currentIndex:%d", static_cast<int>(currentIndex));
        ImGui::Text("sequenceSize:%d", static_cast<int>(steps.size()));

#endif // USE_IMGUI
    }
};


//�ҋ@�X�e�b�v
class WaitStep : public IStep {
    float duration;
    float timer;

public:
    WaitStep(float duration) : duration(duration), timer(0) {}

    void Start(GameObject* owner) override {
        timer = duration;
    }

    void Update(float elapsedTime) override {
        timer -= elapsedTime;
    }

    bool IsFinished() const override {
        return timer <= 0.0f;
    }
};

//�֐��Ăяo���X�e�b�v
class CallStep : public IStep {
    std::function<void(GameObject*)> func;
    bool called = false;

public:
    CallStep(std::function<void(GameObject*)> func) : func(func) {}

    void Start(GameObject* owner) override {
        if (!called) {
            func(owner);
            called = true;
        }
    }

    void Update(float elapsedTime) override {}
    bool IsFinished() const override { return called; }

    void Reset() override {
        called = false;
    }
};

//�����𖞂����܂őҋ@
class ConditionalStep : public IStep {
    std::function<bool()> condition;

public:
    ConditionalStep(std::function<bool()> condition)
        : condition(condition) {}

    void Start(GameObject* owner) override {}

    void Update(float elapsedTime) override {}

    bool IsFinished() const override {
        return condition();
    }
};