#pragma once

#include "../../Engine/Scene/Scene.h"

#include "../GameObject.h"
#include "Dialog.h"
#include "stdUtiles.h"
#include "../UIFactory.h"
#include "../AudioSource.h"

class EditorGUI
{
public:
	static void DrawMainMenu() {

#ifdef USE_IMGUI
		Scene* scene = Scene::GetCurrentScene();
		//上のメニューバー
		if (ImGui::BeginMainMenuBar()) {
			//生成メニュー
			if (ImGui::BeginMenu("UIObject")) {
				//if (ImGui::MenuItem("CreateEmpty")) {
				//	UIFactory::Create("GameObject");
				//}
				if (ImGui::BeginMenu("UI")) {
					if (ImGui::MenuItem("Button")) {
						UIFactory::CreateButton("Button");
					}
					if (ImGui::MenuItem("Image")) {
						UIFactory::CreateImage("Image");
					}
					if (ImGui::MenuItem("Canvas")) {
						UIFactory::CreateCanvas("Canvas");
					}
					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("AudioSource")) {
					static const char* filter = "Audio Files(*.wav;\0*.wav;\0All Files(*.*)\0*.*;\0\0)";
					static int count = 0;
					char filePath[256] = { 0 };
					HWND hwnd = Graphics::GetWindowHandle();
					DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
					if (result == DialogResult::OK) {
						GameObject* obj = UIFactory::Create("AudioSource");
						std::wstring wpath = StringToWstring(std::string(filePath));
						obj->AddComponent<AudioSource>(wpath.c_str());
					}
				}

				ImGui::EndMenu();
			}
			
			ImGui::EndMainMenuBar();
		}
#endif // USE_IMGUI
	}
};