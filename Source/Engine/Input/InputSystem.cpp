#include "InputSystem.h"
#include <Windows.h>
#include "Graphics/Core/Graphics.h"

// 仮想的な左スティック方向のキーコード
#define GAMEPAD_AXIS_UP     0
#define GAMEPAD_AXIS_LEFT   1
#define GAMEPAD_AXIS_DOWN   2
#define GAMEPAD_AXIS_RIGHT  3

// 斜めの入力を正規化
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

    // 0 ~ 1 にスケーリング
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
        if (vkey_ <= 3) // 仮想キー: 左スティック方向（WASD用）
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

//  初期化
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


    inputKeys["W"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_UP));     // スティック上
    inputKeys["A"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_LEFT));   // スティック左
    inputKeys["S"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_DOWN));   // スティック下
    inputKeys["D"].emplace_back(std::make_unique<Gamepad>(GAMEPAD_AXIS_RIGHT));  // スティック右

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


// 更新処理
void InputSystem::Update(float deltaTime)
{
    DWORD xinputResult = XInputGetState(static_cast<DWORD>(slot), &state);
    isGamePadConnected = (xinputResult == ERROR_SUCCESS);

    //入力情報の更新
    {
        if (isGamePadConnected)
        {
            //ゲームパッドのAxisLeft更新
            ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbLX), static_cast<float>(state.Gamepad.sThumbLY),
                deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE),
                mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Left)][static_cast<size_t>(Axis::Y)]);
            //ゲームパッドのAxisRight更新
            ApplyStickDeadzone(static_cast<float>(state.Gamepad.sThumbRX), static_cast<float>(state.Gamepad.sThumbRY),
                deadZoneMode, 32767.f, static_cast<float>(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE),
                mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::X)], mAxis[static_cast<size_t>(Side::Right)][static_cast<size_t>(Axis::Y)]);
        }
        else
        {
            //移動キー更新処理
            for (auto& keys : directionKeys) {
                for (auto& key : keys) {
                    key->Update(deltaTime);
                }
            }

            //値更新
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
        //ボタンの入力更新処理
        for (auto& actionKeys : inputKeys) {
            for (auto& key : actionKeys.second) {
                key->Update(deltaTime);
            }
        }

    }
    // カーソル位置の取得
    POINT cursor;
    ::GetCursorPos(&cursor);
    ScreenToClient(Graphics::GetWindowHandle(), &cursor);

    // マウス座標更新
    mousePositionX[1] = mousePositionX[0];
    mousePositionY[1] = mousePositionY[0];
    mousePositionX[0] = (LONG)cursor.x;
    mousePositionY[0] = (LONG)cursor.y;

    //アクティブデバイス判定

    //コントローラー
    {
        auto buttons = state.Gamepad.wButtons;
        auto lx = state.Gamepad.sThumbLX;
        auto ly = state.Gamepad.sThumbLY;
        // ボタンが押された or スティックが動いたらアクティブデバイスを切り替え
        if (buttons != 0 || abs(lx) > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || abs(ly) > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) {
            activeDevice = InputDeviceType::Gamepad;
        }
    }
    //キーボード
    {
        for (int vk = 0x08; vk <= 0xFE; ++vk) {
            if (GetAsyncKeyState(vk) & 0x8000) {
                activeDevice = InputDeviceType::Keyboard;
            }
        }
    }
    //マウス
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