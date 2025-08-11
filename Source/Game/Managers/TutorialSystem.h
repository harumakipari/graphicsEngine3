#pragma once
#include <functional>
#include "Components/Audio/AudioSourceComponent.h"
#include "Widgets/ObjectManager.h"
#include "Widgets/GameObject.h"

enum class TutorialStep
{
	Start,
	
	Move,			//�ړ��~�b�V����
	Collect,		//����~�b�V����
	FirstAttack,	//���˃~�b�V����
	ManyCollect,	//�`���[�W�~�b�V����
	SecondAttack,	//�ł�����A�G�l���M�[�~�ςŋ@���̓_�E��(Info)
	CreateBuild,	//�Ə��~�b�V���� (�r���ϋv�l���邱�Ƃ�������)
	ManyCollect2,	//�j��~�b�V���� �A�C�e����5�W�߂���
	ThirdAttack,	//�r�����Ռ��g�ŉ󂷗��߂̃r�[��
	BreakBuilds,	//�j��~�b�V����
	MoveCamera,		// �J�����𓮂���
	BossBuild,		//�x�� �{�X�r���o���ďI��

	Finish,
	EnumCount
};

class TutorialSystem
{
	//�`���[�g���A���X�e�b�v
	struct Step
	{
		bool isCompleted = false;
		int setCounter = 0;
		int counter = 0;
		float delay = 0.f;
		std::function<void()> initializeFunction;
		std::function<void()> completedFunction;
		std::function<void()> continueStepFunction;

		//�p�����[�^���Z�b�g
		void Reset() {
			isCompleted = false;
			counter = 0;
			delay = 0.f;
			completedFunction = nullptr;
		}
	};
	static inline Step steps[static_cast<size_t>(TutorialStep::EnumCount)];
	static inline Step presets[static_cast<size_t>(TutorialStep::EnumCount)];
	static inline TutorialStep currentStep;
public:
	//����������
	static void Initialize();

	//�X�V����
	static void Update(float deltaTime);

	//GUI�`��
	static void DrawGUI();

	//���݂̃X�e�b�v�擾
	static TutorialStep GetCurrentStep() { return currentStep; }

	//�X�e�b�v�蓮�ݒ�
	static void SetCurrentStep(TutorialStep step) { 
		if (currentStep == step) {
			if (steps[static_cast<size_t>(step)].continueStepFunction) {
				steps[static_cast<size_t>(step)].continueStepFunction();
			}
		}
		currentStep = step;
		steps[static_cast<size_t>(step)] = presets[static_cast<size_t>(step)];
		if (steps[static_cast<size_t>(step)].initializeFunction) {
			steps[static_cast<size_t>(step)].initializeFunction();
		}

		//if (GameObject* check = ObjectManager::Find("Check"))
		//{
		//	check->SetActive(false);
		//}

		//���Đ�
		switch (step)
		{
		case TutorialStep::Start:
		case TutorialStep::Finish:
			Audio::PlayOneShot(L"./Data/Sound/SE/tutorial_popup.wav");
			break;
		default:
			break;
		}
	}

	//�A�N�V�����ʒm
	static void AchievedAction(TutorialStep step) {
		if (currentStep == step) 
		{
			steps[static_cast<size_t>(step)].counter--;
			if (steps[static_cast<size_t>(step)].counter <= 0 && !steps[static_cast<size_t>(step)].isCompleted)
			{
				steps[static_cast<size_t>(step)].isCompleted = true;
				//���������Ƃ��ɌĂяo��
				if (steps[static_cast<size_t>(step)].completedFunction) 
				{
					steps[static_cast<size_t>(step)].completedFunction();
				}

				//if (GameObject* checkFrame = ObjectManager::Find("CheckFrame"))
				//{
				//	if (checkFrame->IsActive())
				//	{
				//		ObjectManager::Find("Check")->SetActive(true);
				//	}
				//}

				//���Đ�
				switch (step)
				{
				case TutorialStep::Move:
				case TutorialStep::Collect:
				case TutorialStep::FirstAttack:
				case TutorialStep::ManyCollect:
				case TutorialStep::SecondAttack:
				case TutorialStep::CreateBuild:
				case TutorialStep::ManyCollect2:
				case TutorialStep::ThirdAttack:
				case TutorialStep::BreakBuilds:
				case TutorialStep::MoveCamera:
				case TutorialStep::BossBuild:
					//������
					Audio::PlayOneShot(L"./Data/Sound/SE/task_clear.wav");
					break;
				default:
					break;
				}
			}
		}
	}

	//�w��̃X�e�b�v���J�n�����Ƃ��ɌĂяo���֐��ݒ�
	static void SetInitializeFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].initializeFunction = func;
	}

	//�w��̃X�e�b�v�����������Ƃ��ɌĂяo���֐��ݒ�
	static void SetCompletedFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].completedFunction = func;
	}
	//�w��̃X�e�b�v���ĂуZ�b�g���ꂽ�Ƃ��ɌĂяo���֐��ݒ�
	static void SetContinueStepFunction(TutorialStep step, std::function<void()> func) {
		presets[static_cast<size_t>(step)].continueStepFunction = func;
	}

	//�J�E���^�[�擾
	static int GetCounter(TutorialStep step) {
		return steps[static_cast<size_t>(step)].counter;
	}
private:
	//�X�e�b�v�I�v�V�����ݒ�
	static void SetOption(TutorialStep step, int counter, float nextStepDelay = 0.0f) {
		presets[static_cast<size_t>(step)].counter = counter;
		presets[static_cast<size_t>(step)].delay = nextStepDelay;
	}
};