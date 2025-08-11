#pragma once
#include <memory>

class Camera;

class CameraManager
{
public:
    static void ToggleCamera()
    {
        useDebugCamera = !useDebugCamera;
    }

    static Camera* GetCurrentCamera()
    {
        return useDebugCamera ? debugCamera.lock().get() : gameCamera;
    }

    static bool IsUseDebug() { return useDebugCamera; }

    static void SetGameCamera(Camera* camera) { gameCamera = camera; }
    static void SetDebugCamera(std::shared_ptr<Camera> camera) { debugCamera = camera; }
private:
    static inline Camera* gameCamera;
    static inline std::weak_ptr<Camera> debugCamera;

    static inline bool useDebugCamera = false;
};
