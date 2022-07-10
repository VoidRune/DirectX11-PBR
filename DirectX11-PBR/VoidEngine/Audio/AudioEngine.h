#pragma once
#include <string>
#include <xaudio2.h>
#include <x3daudio.h>
#include "Utility/Types.h"
#pragma comment(lib,"xaudio2.lib")

struct Sound
{
private:
	friend class AudioEngine;

	WAVEFORMATEXTENSIBLE wfx = { 0 };
	XAUDIO2_BUFFER buffer = { 0 };

	uint32_t lastVoiceId = -1;
	bool loaded = false;
};

class AudioEngine
{
public:
	/*Returns the static instance							*/
	static inline AudioEngine& Get() { return *s_Instance; }
	/* Channels can have separate volumes					*/
	AudioEngine();
	void Init(int numOfChannel = 16, int numOfVoices = 32);
	~AudioEngine();

	void Play2D(Sound* sound, const int& channel);
	/* +z is sound from right								*/
	/* -z is sound from left								*/
	/* innerRadius is blending point between channels		*/
	void Play3D(Sound* sound, const int& channel, vec3f direction, float innerRadius = 3.0f);
	/* +z is sound from right								*/
	/* -z is sound from left								*/
	/* innerRadius is blending point between channels		*/
	void Update3D(Sound* sound, const int& channel, vec3f direction, float innerRadius = 3.0f);

	/* Stops the last instance of this sound being played	*/
	void Stop(Sound* sound);
	/* Sets the volume to desired channel					*/
	/* Number of channels is defined in the constructor		*/
	void SetVolume(float volume, const int& channel);
	/* Sets the master volume								*/
	/* Final volume is master volume * channel volume		*/
	void SetMasterVolume(float volume);

	/* Stops all currently playing sounds					*/
	/* and flushes all buffers								*/
	void Flush();

	/* Loads the sound in memory							*/
	/* Can load .wav .mp3 .ogg files						*/
	void LoadData(Sound* sound, std::string filename, bool looping = false);

	/* Returns pointer to sound on heap						*/
	/* Can load .wav .mp3 .ogg files						*/
	Sound* LoadData(std::string filename, bool looping = false);

	AudioEngine(const AudioEngine&) = delete;
	AudioEngine& operator=(const AudioEngine&) = delete;
private:
	void Calculate3DMatrix(int channels, float* matrix, vec3f direction, float innerRadius);
	void LoadWav(Sound* sound, std::string filename, bool looping = false);
	void LoadMp3(Sound* sound, std::string filename, bool looping = false);
	void LoadOgg(Sound* sound, std::string filename, bool looping = false);

	static AudioEngine* s_Instance;
};