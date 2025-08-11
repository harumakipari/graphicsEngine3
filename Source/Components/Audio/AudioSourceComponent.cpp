#include "AudioSourceComponent.h"

#include <Windows.h>
#include <winerror.h>

#ifdef X3DAUDIO
#include "../../Core/Actor.h"  
#endif // X3DAUDIO


#ifdef USE_IMGUI
#include <imgui.h>
#include "../../Widgets/Utils/Dialog.h"
#include "../../Widgets/Utils/stdUtiles.h"
#endif // USE_IMGUI


static HRESULT FindChunk(HANDLE hfile, DWORD fourcc, DWORD& chunkSize, DWORD& chunkDataPosition)
{
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD offset = 0;

	while (hr == S_OK)
	{
		DWORD numberOfBytesRead;
		if (0 == ReadFile(hfile, &chunkType, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (0 == ReadFile(hfile, &chunkDataSize, sizeof(DWORD), &numberOfBytesRead, NULL))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch (chunkType)
		{
		case 'FFIR'://RIFF
			riffDataSize = chunkDataSize;
			chunkDataSize = 4;
			if (0 == ReadFile(hfile, &fileType, sizeof(DWORD), &numberOfBytesRead, NULL))
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;
		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, chunkDataSize, NULL, FILE_CURRENT))
			{
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		offset += sizeof(DWORD) * 2;

		if (chunkType == fourcc)
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = offset;
			return S_OK;
		}

		offset += chunkDataSize;

		if (bytesRead >= riffDataSize)
		{
			return S_FALSE;
		}
	}

	return S_OK;
}

static HRESULT ReadChunkData(HANDLE hfile, LPVOID buffer, DWORD bufferSize, DWORD bufferOffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, bufferOffset, NULL, FILE_BEGIN))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}
	DWORD numberOfBytesRead;
	if (0 == ReadFile(hfile, buffer, bufferSize, &numberOfBytesRead, NULL))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}
	return hr;
}

void Audio::CreateAudioSource(std::shared_ptr<AudioBuffer> buffer,
	IXAudio2SourceVoice** sourceVoice, SoundType type)
{
	// BGM,SEの送信先を設定
	XAUDIO2_SEND_DESCRIPTOR send[SoundType::EnumCount] = { { 0, submixVoices[BGM] }, { 0, submixVoices[SE] } };
	XAUDIO2_VOICE_SENDS sends[SoundType::EnumCount] = { {1, &send[BGM]}, {1, &send[SE]} };

	HRESULT hr = xaudio2->CreateSourceVoice(sourceVoice, (WAVEFORMATEX*)&buffer->wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, nullptr, &sends[type], nullptr);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void Audio::Initialize()
{
	HRESULT hr;

	//XAudio2の初期化
	hr = XAudio2Create(xaudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = xaudio2->CreateMasteringVoice(&masterVoice);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// BGMとSE用のサブミキサーボイスを作成（ステレオ: 2チャンネル）
	hr = xaudio2->CreateSubmixVoice(&submixVoices[BGM], 2, 44100);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = xaudio2->CreateSubmixVoice(&submixVoices[SE], 2, 44100);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

#ifdef X3DAUDIO
	//X3DAudioの初期化
	DWORD channelMask;
	masterVoice->GetChannelMask(&channelMask);

	X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, x3dAudioHandle);
#endif // X3DAUDIO
}

Audio::~Audio()
{
	masterVoice->DestroyVoice();
	submixVoices[0]->DestroyVoice();
	submixVoices[1]->DestroyVoice();
	xaudio2->Release();
}

std::shared_ptr<Audio::AudioBuffer> Audio::AudioBuffer::GetResource(const wchar_t* filePath)
{
	//すでに存在していたらリソースを返す
	if (resources.contains(filePath) && !resources.at(filePath).expired())
	{
		return resources.at(filePath).lock();
	}

	//存在してなかったら新規でオーディオバッファを生成
	{
		HRESULT hr;
		std::shared_ptr<AudioBuffer> audioBuffer = std::make_shared<AudioBuffer>();

		//ファイルオープン
		HANDLE hfile = CreateFileW(filePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (INVALID_HANDLE_VALUE == hfile)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		if (INVALID_SET_FILE_POINTER == SetFilePointer(hfile, 0, NULL, FILE_BEGIN))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}

		DWORD chunkSize;
		DWORD chunkPosition;
		//check the file type, should be 'WAVE' or 'XWMA'
		FindChunk(hfile, 'FFIR'/*RIFF*/, chunkSize, chunkPosition);
		DWORD fileType = 0;
		ReadChunkData(hfile, &fileType, sizeof(DWORD), chunkPosition);
		_ASSERT_EXPR(fileType == 'EVAW'/*WAVE*/, L"Only support 'WAVE'");

		FindChunk(hfile, ' tmf'/*FMT*/, chunkSize, chunkPosition);
		ReadChunkData(hfile, &audioBuffer->wfx, chunkSize, chunkPosition);

		//fill out the audio data buffer with the contents of the fourccDATA chunk
		FindChunk(hfile, 'atad'/*DATA*/, chunkSize, chunkPosition);
		BYTE* data = new BYTE[chunkSize];
		ReadChunkData(hfile, data, chunkSize, chunkPosition);

		audioBuffer->buffer.AudioBytes = chunkSize;  //size of the audio buffer in bytes
		audioBuffer->buffer.pAudioData = data; //buffer containing audio data
		audioBuffer->buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

		resources[filePath] = audioBuffer;
		return audioBuffer;
	}
}

void Audio::PlayOneShot(const wchar_t* filePath, float volume)
{
	std::shared_ptr<StandaloneAudioSource> source = std::make_shared<StandaloneAudioSource>(filePath);
	source->SetVolume(volume);
	source->Play();
	audioSources.emplace_back(source);
}

void Audio::Update(float deltaTime)
{
	//破棄処理
	for (auto& source : audioSources)
	{
		if (!source->IsPlaying())
		{
			erases.emplace_back(source);
		}
	}
	audioSources.erase(std::remove_if(audioSources.begin(), audioSources.end(),
		[&](const auto& audioSource) {
			return std::find(erases.begin(), erases.end(), audioSource) != erases.end();
		}), 
		audioSources.end());
	erases.clear();
}

void AudioSourceComponent::SetSource(const wchar_t* filePath)
{
	this->type = std::wstring(filePath).find(L"BGM") != std::wstring::npos ? SoundType::BGM : SoundType::SE;
	this->filePath = filePath;
	sptrBuffer = Audio::AudioBuffer::GetResource(filePath);
	Audio::CreateAudioSource(Audio::AudioBuffer::GetResource(this->filePath), &sourceVoice, type);
}
AudioSourceComponent::~AudioSourceComponent()
{
	Stop();
	sourceVoice->DestroyVoice();
}

void AudioSourceComponent::Play(int loopCount)
{
	_ASSERT_EXPR(sourceVoice, L"ソースが設定されていません。");

	HRESULT hr;

	Stop();

	XAUDIO2_BUFFER* pBuffer = &sptrBuffer->buffer;

	pBuffer->LoopCount = loopCount;
	hr = sourceVoice->SubmitSourceBuffer(pBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void AudioSourceComponent::Stop(bool playTails, bool waitForBufferToUnqueue)
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

void AudioSourceComponent::SetVolume(float volume)
{
	HRESULT hr = sourceVoice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

uint32_t AudioSourceComponent::GetBufferQueueCount()
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued;
}

void AudioSourceComponent::Tick(float deltaTime)
{
	
#ifdef X3DAUDIO
	if (!AudioListenerComponent::currentActor.expired())
	{
		std::shared_ptr<Actor> listenerActor = AudioListenerComponent::currentActor.lock();
		X3DAUDIO_LISTENER listener{};
		listener.OrientFront = { 0,0,1 };
		listener.OrientTop = { 0,1,0 };
		listener.Position = listenerActor->GetPosition();

		X3DAUDIO_EMITTER emitter{};
		emitter.Position = GetComponentLocation();
		emitter.ChannelCount = 1;
		emitter.CurveDistanceScaler = 1.0f;//カーブのスケール
		emitter.pVolumeCurve = nullptr; //nullならデフォルトのカーブ（距離によって減衰）

		X3DAUDIO_DSP_SETTINGS dspSettings{};
		FLOAT32 matrix[XAUDIO2_MAX_AUDIO_CHANNELS * 1] = {};
		dspSettings.SrcChannelCount = 1;
		dspSettings.DstChannelCount = 2;//ステレオ環境なら２
		dspSettings.pMatrixCoefficients = matrix;

		X3DAudioCalculate(Audio::x3dAudioHandle, &listener, &emitter,
			X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER,
			&dspSettings);

		sourceVoice->SetOutputMatrix(Audio::masterVoice, 1, 2, matrix);
		sourceVoice->SetFrequencyRatio(dspSettings.DopplerFactor); //ドップラー効果
	}
#endif // X3DAUDIO

	
}

void AudioSourceComponent::DrawImGuiInspector()
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



StandaloneAudioSource::StandaloneAudioSource(const wchar_t* filePath)
{
	this->type = std::wstring(filePath).find(L"BGM") != std::wstring::npos ? SoundType::BGM : SoundType::SE;
	sptrBuffer = Audio::AudioBuffer::GetResource(filePath);
	Audio::CreateAudioSource(sptrBuffer, &sourceVoice, type);
}
StandaloneAudioSource::~StandaloneAudioSource()
{
	Stop();
	sourceVoice->DestroyVoice();
}

void StandaloneAudioSource::Play(bool loop)
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

	pBuffer->LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;
	hr = sourceVoice->SubmitSourceBuffer(pBuffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = sourceVoice->Start(0);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

void StandaloneAudioSource::Stop(bool playTails)
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
}

void StandaloneAudioSource::SetVolume(float volume)
{
	HRESULT hr = sourceVoice->SetVolume(volume);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}

float StandaloneAudioSource::GetVolume()
{
	float volume;
	sourceVoice->GetVolume(&volume);
	return volume;
}

bool StandaloneAudioSource::IsPlaying()
{
	XAUDIO2_VOICE_STATE voiceState{};
	sourceVoice->GetState(&voiceState);
	return voiceState.BuffersQueued > 0;
}

void StandaloneAudioSource::DrawGUI()
{
#ifdef USE_IMGUI
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