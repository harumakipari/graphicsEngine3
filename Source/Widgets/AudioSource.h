#pragma once
#include "../Components/Audio/AudioSourceComponent.h"
#include "UIComponent.h"

class AudioSource : public UIComponent
{
public:
	AudioSource(const wchar_t* filePath) {
		SetSource(filePath);
	}
	virtual ~AudioSource() override;

	/// <summary>
	/// �\�[�X�ݒ�
	/// </summary>
	/// <param name="filePath">�t�@�C���p�X</param>
	void SetSource(const wchar_t* filePath);

	void Play(int loopCount = 0);
	void Stop(bool playTails = true, bool waitForBufferToUnqueue = true);
	void SetVolume(float volume);

	void SetLoopOption(float begin, float length)
	{
		XAUDIO2_BUFFER* pBuffer = &sptrBuffer->buffer;
		const WAVEFORMATEX& format = sptrBuffer->wfx.Format;

		const UINT32 sampleRate = format.nSamplesPerSec;
		const UINT32 blockAlign = format.nBlockAlign;
		const UINT32 totalSamples = pBuffer->AudioBytes / blockAlign;

		UINT32 loopBegin = static_cast<UINT32>(sampleRate * begin);
		UINT32 loopLength = static_cast<UINT32>(sampleRate * length);

		// �͈̓`�F�b�N
		if (loopBegin >= totalSamples) {
			loopBegin = 0;
			loopLength = 0;
			pBuffer->LoopCount = 0; // ������
		}
		else if (loopBegin + loopLength > totalSamples) {
			loopLength = totalSamples - loopBegin;
		}

		pBuffer->LoopBegin = loopBegin;
		pBuffer->LoopLength = loopLength;
	}

	/// <summary>
	/// �o�b�t�@�[�L���[�擾
	/// </summary>
	/// <returns>�i�Đ����Ȃ�O���傫���j</returns>
	uint32_t GetBufferQueueCount();

	void DrawProperty() override;

private:
	const wchar_t* filePath = nullptr;
	SoundType type;

	std::shared_ptr<Audio::AudioBuffer> sptrBuffer;
	IXAudio2SourceVoice* sourceVoice;

	static inline float masterVolume = 1.0f;
	static inline float bgmVolume = 1.0f;
	static inline float seVolume = 1.0f;
};