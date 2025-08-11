#include "InputSystem.h"
#include <Windows.h>
#include "Graphics/Core/Graphics.h"

// ���z�I�ȍ��X�e�B�b�N�����̃L�[�R�[�h
#define GAMEPAD_AXIS_UP     0
#define GAMEPAD_AXIS_LEFT   1
#define GAMEPAD_AXIS_DOWN   2
#define GAMEPAD_AXIS_RIGHT  3

// �΂߂̓��͂𐳋K��
static float ApplyLinearDeadzone(float value, float maxValue, float deadZoneSize)
{
    if (value < -deadZoneSize)
    {
        value += deadZoneSize;
    }
    else if (value > deadZoneSize)
    {
        value -= deadZoneSize;
    }
    else
    {
        return 0;
    }

    // 0 ~ 1 �ɃX�P�[�����O
    float scaledValue = value / (maxValue - deadZoneSize);
    return std::max<float>(-1.0f, std::min<float>(scaledValue, 1.0f));
}

static void ApplyStickDeadzone(float x, float y, DeadZoneMode deadZoneMode, float maxValue, float deadZoneSize, _Out_ float& resultX, _Out_ float& resultY)
{
    switch (deadZoneMode)
    {
    case DeadZoneMode::IndependentAxes:
        resultX = ApplyLinearDeadzone(x, maxValue, deadZoneSize);
        resultY = ApplyLinearDeadzone(y, maxValue, deadZoneSize);
        break;
    case  DeadZoneMode::Circular:
    {
        float dist = sqrtf(x * x + y * y);
        float wanted = ApplyLinearDeadzone(dist, maxValue, deadZoneSize);
        float scale = (wanted > 0.0f) ? (wanted / dist) : 0.0f;
        resultX = std::max<float>(-1.0f, std::min<float>(x * scale, 1.0f));
        resultY = std::max<float>(-1.0f, std::min<float>(y * scale, 1.0f));
        break;
    }
    default: //DeadZoneMode::None:
        //resultX
        break;
    }
}

void InputKey::Update(float deltaTime)
{
    oldPressTime_ = pressTime_;
    pressTime_ = (static_cast<USHORT>(GetAsyncKeyState(vkey_)) & 0x8000) ? pressTime_ + deltaTime : 0.0f;
}

void Gamepad::Update(float deltaTime)
{
    oldPressTime_ = pressTime_;
    _XINPUT_STATE state = InputSystem::GetXInputState();
    const auto& pad = state.Gamepad;
    switch (keyType)
    {
    case GamePadKeyType::Key:
#if 1
        if (vkey_ <= 3) // ���z�L�[: ���X�e�B�b�N�����iWASD�p�j
        {
            float lx = pad.sThumbLX / 32767.0f;
            float ly = pad.sThumbLY / 32767.0f;
            float deadZone = 0.25f;

            bool active = false;
            switch (vkey_)
            {
            case GAMEPAD_AXIS_UP:    active = (ly > deadZone);  break;  // W
            case GAMEPAD_AXIS_LEFT:  active = (lx < -deadZone); break;  // A
            case GAMEPAD_AXIS_DOWN:  active = (ly < -deadZone); break;  // S
            case GAMEPAD_AXIS_RIGHT: active = (lx > deadZone);  break;  // D
            }

            pressTime_ = active ? pressTime_ + deltaTime : 0.0f;
        }
        else
        {
            pressTime_ = (pad.wButtons & vkey_) ? pressTime_ + deltaTime : 0.0f;
        }
#else
        pressTime_ = (state.Gamepad.wButtons & vkey_) ? pressTime_ + deltaTime : 0.0f;
#endif
        break;
    case GamePadKeyType::LeftTrigger:
        pressTime_ = (state.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? pressTime_ + deltaTime : 0.0f;
        break;
    case GamePadKeyType::RightTrigger:
        pressTime_ = (state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) ? pressTime_ + deltaTime : 0.0f;
        break;
    default:
        int a = 0;
        break;
    }

}

//  ������
void InputSystem::Initialize()
{
    directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Up)] = std::make_unique<Keyboard>('W');
    directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Left)] = std::make_unique<Keyboard>('A');
    directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Down)] = std::make_unique<Keyboard>('S');
    directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Right)] = std::make_unique<Keyboard>('D');

    directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Up)] = std::make_unique<Keyboard>('I');
    directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Left)] = std::make_unique<Keyboard>('J');
    directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Down)] = std::make_unique<Keyboard>('K');
    directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Right)] = std::make_unique<Keyboard>('L');


    inputKeys.clear();

    inputKeys["MouseRight"].emplace_back(std::make_unique<Mouse>(VK_RBUTTON));
    inputKeys["MouseLeft"].emplace_back(std::make_unique<Mouse>(VK_LBUTTON));


    inputKeys["F8"].emplace_back(std::make_unique<Keyboard>(VK_F8));
    inputKeys["Alt"].emplace_back(std::make_unique<Keyboard>(VK_MENU));
    inputKeys["Enter"].emplace_back(std::make_unique<Keyboard>(VK_RETURN));
    inputKeys["Shift"].emplace_back(std::make_unique<Keyboard>(VK_SHIFT));

    inputKeys["Space"].emplace_back(std::make_unique<Keyboard>(VK_SPACE));
    inputKeys["Space"].emplace_back(std::make_unique<Gamepad>(XINPUT_GAMEPAD_X));

    inputKeys["Up"].emplace_back(std::make_unique<Keyboard>(VK_UP));
    inputKeys["W"].emplace_back(std::make_unique<Keyboard>('W'));
    inputKeys["Left"].emplace_back(std::make_unique<Keyboard>(VK_LEFT));
    inputKeys["A"].emplace_back(std::make_unique<Keyboard>('A'));
    inputKeys["Down"].emplace_back(std::make_unique<Keyboard>(VK_DOWN));
    inputKeys["S"].emplace_back(std::make_unique<Keyboard>('S'));
    inputKeys["Right"].emplace_back(std::make_unique<Keyboard>(VK_RIGHT));
    inputKeys["D"].emplace_back(std::make_unique<Keyboard>('D'));


    inputKeys["W"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_UP));     // �X�e�B�b�N��
    inputKeys["A"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_LEFT));   // �X�e�B�b�N��
    inputKeys["S"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_DOWN));   // �X�e�B�b�N��
    inputKeys["D"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_RIGHT));  // �X�e�B�b�N�E

    inputKeys["E"].emplace_back(std::make_unique<Keyboard>('E'));
    inputKeys["Q"].emplace_back(std::make_unique<Keyboard>('Q'));
    inputKeys["Z"].emplace_back(std::make_unique<Keyboard>('Z'));
    inputKeys["R"].emplace_back(std::make_unique<Keyboard>('R'));
    inputKeys["X"].emplace_back(std::make_unique<Keyboard>('X'));
    inputKeys["T"].emplace_back(std::make_unique<Keyboard>('T'));
    inputKeys["Y"].emplace_back(std::make_unique<Keyboard>('Y'));
    inputKeys["Z"].emplace_back(std::make_unique<Keyboard>('Z'));
    inputKeys["Backspace"].emplace_back(std::make_unique<Keyboard>(VK_BACK));
    inputKeys["Backspace"].emplace_back(std::make_unique<Keyboard>(XINPUT_GAMEPAD_B));
    inputKeys["Enter"].emplace_back(std::make_unique<Gamepad>(XINPUT_GAMEPAD_A));

    inputKeys["ok"].emplace_back(std::make_unique<Mouse>(VK_LBUTTON));
    inputKeys["ok"].emplace_back(std::make_unique<Keyboard>(VK_RETURN));
    inputKeys["ok"].emplace_back(std::make_unique<Gamepad>(XINPUT_GAMEPAD_A));

}


// �X�V����
void InputSystem::Update(float deltaTime)
{
    DWORD xinputResult = XInputGetState(static_cast<DWORD>(slot), &state);
    isGamePadConnected = (xinputResult == ERROR_SUCCESS);

    //���͏��̍X�V
    {
        if (isGamePadConnected)
        {
            //�Q�[���p�b�h��AxisLeft�X�V
            ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbLX), static_cast<float>(state.Gamepad.sThumbLY),
                deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
                mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);
            //�Q�[���p�b�h��AxisRight�X�V
            ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbRX), static_cast<float>(state.Gamepad.sThumbRY),
                deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
                mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::Y)]);
        }
        else
        {
            //�ړ��L�[�X�V����
            for (auto& keys : directionKeys) {
                for (auto& key : keys) {
                    key->Update(deltaTime);
                }
            }

            //�l�X�V
            ApplyStickDeadzone(
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Right)]->IsPressed()) -
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Left)]->IsPressed()),
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Up)]->IsPressed()) -
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Left)][static_cast<size_t>(Direction::Down)]->IsPressed()),
                deadZoneMode, 1.f, 0.f,
                mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);

            ApplyStickDeadzone(
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Right)]->IsPressed()) -
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Left)]->IsPressed()),
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Up)]->IsPressed()) -
                static_cast<float>(directionKeys[static_cast<size_t>(Side::Right)][static_cast<size_t>(Direction::Down)]->IsPressed()),
                deadZoneMode, 1.f, 0.f,
                mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::Y)]);
        }
        //�{�^���̓��͍X�V����
        for (auto& actionKeys : inputKeys) {
            for (auto& key : actionKeys.second) {
                key->Update(deltaTime);
            }
        }

    }
    // �J�[�\���ʒu�̎擾
    POINT cursor;
    ::GetCursorPos(&cursor);
    ScreenToClient(Graphics::GetWindowHandle(), &cursor);

    // �}�E�X���W�X�V
    mousePositionX[1] = mousePositionX[0];
    mousePositionY[1] = mousePositionY[0];
    mousePositionX[0] = (LONG)cursor.x;
    mousePositionY[0] = (LONG)cursor.y;

    //�A�N�e�B�u�f�o�C�X����

    //�R���g���[���[
    {
        auto buttons = state.Gamepad.wButtons;
        auto lx = state.Gamepad.sThumbLX;
        auto ly = state.Gamepad.sThumbLY;
        // �{�^���������ꂽ or �X�e�B�b�N����������A�N�e�B�u�f�o�C�X��؂�ւ�
        if (buttons != 0 || abs(lx) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || abs(ly) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
            activeDevice = InputDeviceType::Gamepad;
        }
    }
    //�L�[�{�[�h
    {
        for (int vk = 0x08; vk <= 0xFE; ++vk) {
            if (GetAsyncKeyState(vk) & 0x8000) {
                activeDevice = InputDeviceType::Keyboard;
            }
        }
    }
    //�}�E�X
    {
        if (GetAsyncKeyState(VK_LBUTTON) & 0x8000 ||
            GetAsyncKeyState(VK_RBUTTON) & 0x8000 ||
            mousePositionX[0] != mousePositionX[1] ||
            mousePositionY[0] != mousePositionY[1]) {
            activeDevice = InputDeviceType::Mouse;
        }
    }
}

bool InputSystem::GetInputState(const std::string& action, InputStateMask state, DeviceFlags flag)
{
    auto it = inputKeys.find(action);
    if (it != inputKeys.end())
    {
        const auto& keys = it->second;
        for (auto& key : keys) {
            switch (flag)
            {
            case DeviceFlags::KeyboardOnly:
                if (key->GetDeviceType() != InputDeviceType::Keyboard) continue;
                break;
            case DeviceFlags::MouseOnly:
                if (key->GetDeviceType() != InputDeviceType::Mouse) continue;
                break;
            case DeviceFlags::GamePadOnly:
                if (key->GetDeviceType() != InputDeviceType::Gamepad) continue;
                break;
            case DeviceFlags::KeyboardAndMouse:
                if (key->GetDeviceType() == InputDeviceType::Gamepad) continue;
                break;
            case DeviceFlags::KeyboardAndGamePad:
                if (key->GetDeviceType() == InputDeviceType::Mouse) continue;
                break;
            case DeviceFlags::MouseAndGamePad:
                if (key->GetDeviceType() == InputDeviceType::Keyboard) continue;
                break;
            }
            switch (state)
            {
            case InputStateMask::Trigger:
                if (key->IsTrigger()) return true;
                break;
            case InputStateMask::Release:
                if (key->IsRelease()) return true;
                break;
            default:
                if (key->IsPressed()) return true;
                break;
            }
        }
    }
    return false;
}