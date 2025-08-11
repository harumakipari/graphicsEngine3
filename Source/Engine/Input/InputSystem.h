#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <Windows.h>

#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

#include <DirectXMath.h>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

enum class InputDeviceType { Keyboard, Mouse, Gamepad };

class InputKey
{
protected:
    int vkey_;
    float pressTime_ = 0.0f;
    float oldPressTime_ = 0.0f;
    InputDeviceType deviceType_;

public:
    InputKey(int vKey, InputDeviceType deviceType) : vkey_(vKey), pressTime_(0), oldPressTime_(0), deviceType_(deviceType) {}
    virtual ~InputKey() = default;
    virtual void Update(float deltaTime);
    virtual bool IsPressed() const { return pressTime_ > 0; }
    virtual bool IsTrigger() const { return (oldPressTime_ == 0 && pressTime_ > 0); }
    virtual bool IsRelease() const { return (pressTime_ == 0 && oldPressTime_ > 0); }

    virtual InputDeviceType GetDeviceType() const { return deviceType_; }
};

class Keyboard :public InputKey
{
public:
    Keyboard(int vkey) :InputKey(vkey, InputDeviceType::Keyboard) {}
    virtual ~Keyboard() override = default;
    Keyboard(Keyboard&) = delete;
    Keyboard& operator=(Keyboard&) = delete;
};

class Mouse :public InputKey
{
public:
    Mouse(int vkey) :InputKey(vkey, InputDeviceType::Mouse) {}
    virtual ~Mouse() override = default;
    Mouse(Mouse&) = delete;
    Mouse& operator=(Mouse&) = delete;
};


///------------  Gamepad ----------///
enum class GamePadKeyType { Key, LeftTrigger, RightTrigger };
enum class Side { Left, Right };
enum class Axis { X, Y };

class Gamepad :public InputKey
{
private:
    GamePadKeyType keyType;
public:
    Gamepad(int vkey, GamePadKeyType type = GamePadKeyType::Key) :InputKey(vkey, InputDeviceType::Gamepad),keyType(type) {}
    virtual ~Gamepad() override = default;
    Gamepad(Gamepad&) = delete;
    Gamepad& operator=(Gamepad&) = delete;

    void Update(float deltaTime) override;
};



enum class DeadZoneMode { IndependentAxes, Circular, None };
enum class DeviceFlags { All, KeyboardOnly, MouseOnly, GamePadOnly, KeyboardAndMouse, KeyboardAndGamePad, MouseAndGamePad };
enum class InputStateMask { None, Trigger, Release };
enum class Direction { Up, Left, Down, Right, None };


class InputSystem
{
private:
    static inline std::unordered_map<std::string, std::vector<std::unique_ptr<InputKey>>> inputKeys;
    static inline std::unique_ptr<InputKey> directionKeys[2][4];
private:
    InputSystem() = default;
    virtual ~InputSystem() = default;

public:
    //  初期化
    static void Initialize();

    //終了化
    static void Finalize(){}

    // 更新処理
    static void Update(float deltaTime);

    // 入力状態の取得
    static bool GetInputState(const std::string& action, InputStateMask state = InputStateMask::None, DeviceFlags flag = DeviceFlags::All);

    // 方向スティック状態の取得
    static float GetAxis(Side side, Axis axis) { return mAxis[static_cast<size_t>(side)][static_cast<size_t>(axis)]; }
    static int GetAxisRaw(Side side, Axis axis) { return static_cast<int>(round(GetAxis(side, axis))); }
    static Direction GetAxisDirection()
    {
        int ax = GetAxisRaw(Side::Left, Axis::X);
        int ay = GetAxisRaw(Side::Left, Axis::Y);
        if (ax == 1) { return Direction::Right; }
        if (ax == -1) { return Direction::Left; }
        if (ay == 1) { return Direction::Up; }
        if (ay == -1) { return Direction::Down; }

        return Direction::None;
    }
    
    static DirectX::XMFLOAT2 GetAxisDirectionVector()
    {
        int ax = GetAxisRaw(Side::Left, Axis::X);
        int ay = GetAxisRaw(Side::Left, Axis::Y);

        return { static_cast<float>(ax),static_cast<float>(ay) };
    }

    // マウスカーソルの移動量取得
    static void GetMouseDelta(int& x, int& y) { (x = mousePositionX[0] - mousePositionX[1], y = mousePositionY[0] - mousePositionY[1]); }

    // マウスカーソルのX座標を取得
    static int GetMousePositionX() { return mousePositionX[0]; }

    // マウスカーソルのY座標を取得
    static int GetMousePositionY() { return mousePositionY[0]; }

    // マウスカーソルの位置を取得
    static DirectX::XMFLOAT2 GetMousePosition()
    {
        DirectX::XMFLOAT2 mousePosition;
        mousePosition.x = static_cast<float>(mousePositionX[0]);
        mousePosition.y = static_cast<float>(mousePositionY[0]);
        return mousePosition;
    }

    // 前回のマウスカーソルX座標取得
    static int GetOldMousePositionX() { return mousePositionX[1]; }

    // 前回のマウスカーソルY座標取得
    static int GetOldMousePositionY() { return mousePositionY[1]; }

    // カーソルが表示されているか
    static bool IsCursolVisible() { return cursolVisible; }

private:

    // カーソルの表示非表示を変更
    static void SetCursolVisible(bool visible)
    {
        cursolVisible = visible;
        int count = 0;

        do
        {
            count = ShowCursor(visible);
        } while ((visible && count < 0) || (!visible && count >= 0));
    }

public:
    // ゲームパッドが接続されているか
    static bool IsGamepadConnected() { return isGamePadConnected; }

    // アクティブなデバイスを取得
    static InputDeviceType GetActiveDevice() { return activeDevice; }

private:
    static inline InputDeviceType activeDevice = InputDeviceType::Keyboard;

    friend class Gamepad;
    static XINPUT_STATE GetXInputState() { return state; }

    static inline float mAxis[2][2];
    static inline XINPUT_STATE state;
    static inline DeadZoneMode deadZoneMode = DeadZoneMode::Circular;
    static inline int slot = 0;

    static inline int mousePositionX[2];
    static inline int mousePositionY[2];

    static inline bool isGamePadConnected = false;

    static inline bool cursolLock = false;
    static inline bool cursolVisible = true;

};

#endif // INPUT_SYSTEM_H