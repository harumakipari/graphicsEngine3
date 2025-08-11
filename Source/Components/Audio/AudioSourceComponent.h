#pragma once
#include <unordered_map>
#include <xaudio2.h>

#ifdef X3DAUDIO
#pragma comment(lib, "X3DAudio.lib")
#include <x3daudio.h>
#endif // X3DAUDIO


#include <wrl.h>

#include <mmreg.h>
#include <memory>

#include "Engine/Utility/Win32Utils.h"

#include "../Base/SceneComponent.h"

enum SoundType
{
	BGM,
	SE,
	EnumCount
};

class AudioSourceComponent;
class StandaloneAudioSource;

class Audio
{
public:
	class AudioBuffer
	{
	public:
		AudioBuffer() = default;
		~AudioBuffer() {
			delete[] buffer.pAudioData;
		}

		WAVEFORMATEXTENSIBLE wfx = { 0 };
		XAUDIO2_BUFFER buffer = { 0 };
	public:
		//オーディオリソース取得
		static std::shared_ptr<AudioBuffer> GetResource(const wchar_t* filePath);
	private:
		friend class Audio;
		static inline std::map<const wchar_t*, std::weak_ptr<AudioBuffer>> resources;
	};

	//初期化処理
	static void Initialize();

	//マスターボリューム設定
	static void SetMasterVolume(float volume) { masterVoice->SetVolume(volume); }
	
	//マスターボリューム取得
	static void GetMasterVolume(float& volume) { masterVoice->GetVolume(&volume); }

	//BGM全体のボリューム設定
	static void SetBgmVolume(float volume) { submixVoices[BGM]->SetVolume(volume); }

	//BGM全体のボリューム取得
	static void GetBgmVolume(float& volume) { submixVoices[BGM]->GetVolume(&volume); }

	//SE全体のボリューム設定
	static void SetSeVolume(float volume) { submixVoices[SE]->SetVolume(volume); }

	//SE全体のボリューム取得
	static void GetSeVolume(float& volume) { submixVoices[SE]->GetVolume(&volume); }

	virtual ~Audio();

public:

	static void PlayOneShot(const wchar_t* filePath, float volume = 1.0f);

	static void Update(float deltaTime);
	static void ClearAll() {
		audioSources.clear();
		erases.clear();
	}
private:
	static inline std::vector<std::shared_ptr<StandaloneAudioSource>> audioSources;
	static inline std::vector<std::shared_ptr<StandaloneAudioSource>> erases;

private:
	friend class AudioSourceComponent;
	friend class AudioSource;
	friend class StandaloneAudioSource;
	static void CreateAudioSource(std::shared_ptr<AudioBuffer> buffer, IXAudio2SourceVoice** sourceVoice, SoundType type);

private:
#ifdef X3DAUDIO
	static inline X3DAUDIO_HANDLE x3dAudioHandle;
#endif // X3DAUDIO
private:
	static inline Microsoft::WRL::ComPtr<IXAudio2> xaudio2;
	static inline IXAudio2MasteringVoice* masterVoice = nullptr;
	static inline IXAudio2SubmixVoice* submixVoices[SoundType::EnumCount];
};


//オーディオソースコンポーネント
class AudioSourceComponent : public SceneComponent
{
public:
	AudioSourceComponent(const std::string& name, std::shared_ptr<Actor> owner) : SceneComponent(name, owner), sourceVoice(nullptr) {}
	virtual ~AudioSourceComponent();

	/// <summary>
	/// ソース設定
	/// </summary>
	/// <param name="filePath">ファイルパス</param>
	void SetSource(const wchar_t* filePath);

	void Play(int loopCount = 0);
	void Stop(bool playTails = true, bool waitForBufferToUnqueue = true);
	void SetVolume(float volume);
	/// <summary>
	/// バッファーキュー取得
	/// </summary>
	/// <returns>（再生中なら０より大きい）</returns>
	uint32_t GetBufferQueueCount();

	void SetLoopOption(float begin, float length)
	{
		XAUDIO2_BUFFER* pBuffer = &sptrBuffer->buffer;
		const WAVEFORMATEX& format = sptrBuffer->wfx.Format;

		const UINT32 sampleRate = format.nSamplesPerSec;
		const UINT32 blockAlign = format.nBlockAlign;
		const UINT32 totalSamples = pBuffer->AudioBytes / blockAlign;

		UINT32 loopBegin = static_cast<UINT32>(sampleRate * begin);
		UINT32 loopLength = static_cast<UINT32>(sampleRate * length);

		// 範囲チェック
		if (loopBegin >= totalSamples) {
			loopBegin = 0;
			loopLength = 0;
			pBuffer->LoopCount = 0; // 無効化
		}
		else if (loopBegin + loopLength > totalSamples) {
			loopLength = totalSamples - loopBegin;
		}

		pBuffer->LoopBegin = loopBegin;
		pBuffer->LoopLength = loopLength;
	}

public:

	void Tick(float deltaTime) override;

	void DrawImGuiInspector() override;

private:
	const wchar_t* filePath = nullptr;
	SoundType type;

	std::shared_ptr<Audio::AudioBuffer> sptrBuffer;
	IXAudio2SourceVoice* sourceVoice;

	static inline float masterVolume = 1.0f;
	static inline float bgmVolume = 1.0f;
	static inline float seVolume = 1.0f;
};

#ifdef X3DAUDIO
//オーディオリスナーコンポーネント
class AudioListenerComponent : public SceneComponent
{
	friend class AudioSourceComponent;
	static inline std::weak_ptr<Actor> currentActor;
public:
	AudioListenerComponent(const std::string& name, std::shared_ptr<Actor> owner) : SceneComponent(name, owner)
	{
		currentActor = owner;
	}
};
#endif // X3DAUDIO

//単体で動くオーディオソース
class StandaloneAudioSource
{
public:
	StandaloneAudioSource(const wchar_t* filePath);
	~StandaloneAudioSource();

	void Play(bool loop = false);
	void Stop(bool playTails = true);
	void SetVolume(float volume);
	float GetVolume();

	bool IsPlaying();

	void DrawGUI();

private:
	SoundType type;

	std::shared_ptr<Audio::AudioBuffer> sptrBuffer;
	IXAudio2SourceVoice* sourceVoice;

	static inline float masterVolume = 1.0f;
	static inline float bgmVolume = 1.0f;
	static inline float seVolume = 1.0f;
};