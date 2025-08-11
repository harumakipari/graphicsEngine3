#include "AudioSource.h"
#include "Utils/Dialog.h"
#include "Utils/stdUtiles.h"

void AudioSource::SetSource(const wchar_t* filePath)
{
	this->type = std::wstring(filePath).find(L"BGM") != std::wstring::npos ? SoundType::BGM : SoundType::SE;
	this->filePath = filePath;
	sptrBuffer = Audio::AudioBuffer::GetResource(filePath);
	Audio::CreateAudioSource(sptrBuffer, &sourceVoice, type);
}
AudioSource::~AudioSource()
{
	Stop();
	sourceVoice->DestroyVoice();
}

void AudioSource::Play(int loopCount)
{
	_ASSERT_EXPR(sourceVoice, L"ソースが設定されていません。");

	HRESULT hr;

	XAUDIO2_VOICE_STATE voiceState = {};
	sourceVoice->GetState(&voiceState);

	if (voiceState.BuffersQueued)
	{
		//Stop(false, 0);
		return;
	}

	XAUDIO2_BUFFER* pBuffer = &sptrBuffer->buffer;

	pBuffer->LoopCount = loopCount;
	hr = sourceVoice->SubmitSourceBuffer(pBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioSource::Stop(bool playTails, bool waitForBufferToUnqueue)
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	if (!voiceState.BuffersQueued)
	{
		return;
	}

	HRESULT hr;
	hr = sourceVoice->Stop(playTails ? XAUDIO2_PLAY_TAILS : 0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = sourceVoice->FlushSourceBuffers();
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	/*while (waitForBufferToUnqueue && voiceState.BuffersQueued)
	{
		sourceVoice->GetState(&voiceState);
	}*/
}

void AudioSource::SetVolume(float volume)
{
	HRESULT hr = sourceVoice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

uint32_t AudioSource::GetBufferQueueCount()
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued;
}

void AudioSource::DrawProperty()
{
#ifdef USE_IMGUI
	
	if (ImGui::Button("Source")) {
		static const char* filter = "Audio Files(*.wav;\0*.wav;\0All Files(*.*)\0*.*;\0\0)";

		char filePath[256] = { 0 };
		HWND hwnd = Graphics::GetWindowHandle();
		DialogResult result = Dialog::OpenFileName(filePath, sizeof(filePath), filter, nullptr, hwnd);
		if (result == DialogResult::OK) {
			std::wstring wpath = StringToWstring(std::string(filePath));
			SetSource(wpath.c_str());
		}
	}

	if (ImGui::Button("Play")) {
		Play();
	}
	if (ImGui::Button("Stop")) {
		Stop();
	}

	static float volume;
	if (sourceVoice)
	{
		sourceVoice->GetVolume(&volume);
		if (ImGui::SliderFloat("Volume", &volume, 0, 1)) {
			sourceVoice->SetVolume(volume);
		}
	}

	ImGui::Separator();

	if (ImGui::SliderFloat("MasterVolume", &masterVolume, 0, 1)) {
		Audio::SetMasterVolume(masterVolume);
	}
	if (ImGui::SliderFloat("BgmVolume", &bgmVolume, 0, 1)) {
		Audio::SetBgmVolume(bgmVolume);
	}
	if (ImGui::SliderFloat("SeVolume", &seVolume, 0, 1)) {
		Audio::SetSeVolume(seVolume);
	}

#endif // USE_IMGUI
}