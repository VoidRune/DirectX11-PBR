#include "AudioEngine.h"

#define MINIMP3_IMPLEMENTATION
#include "Dependencies/minimp3/minimp3.h"
#include "Dependencies/minimp3/minimp3_ex.h"

#include "Dependencies/minivorbis/minivorbis.h"

#include <vector>
#include <filesystem>
#include "Utility/Log.h"

static mp3dec_t s_Mp3d;
AudioEngine* AudioEngine::s_Instance = nullptr;
HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);

struct Voice
{
	uint32_t id;
	IXAudio2SourceVoice* voice;
};
struct Submix
{
	IXAudio2SubmixVoice* submixVoice;
	XAUDIO2_SEND_DESCRIPTOR SFXSend = {};
	XAUDIO2_VOICE_SENDS SFXSendList = {};
};

std::vector<Voice> m_Voices;
std::vector<uint32_t> m_ids;

std::vector<Submix> m_Submix;

IXAudio2* pXAudio2;
X3DAUDIO_HANDLE audio3D;
IXAudio2MasteringVoice* pMasterVoice;

XAUDIO2_VOICE_DETAILS masterVoiceDetails;

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
	HANDLE hBufferEndEvent;
	VoiceCallback() : hBufferEndEvent(CreateEvent(NULL, FALSE, FALSE, NULL)) {}
	~VoiceCallback() { CloseHandle(hBufferEndEvent); }

	//Called when the voice has just finished playing a contiguous audio stream.
	void OnStreamEnd() { SetEvent(hBufferEndEvent); }

	//Unused methods are stubs
	void OnVoiceProcessingPassEnd() override { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) override { }
	void OnBufferEnd(void* pBufferContext) override
	{
		uint32_t id = (uint32_t)pBufferContext;
		m_ids.push_back(id);
	}
	void OnBufferStart(void* pBufferContext) override { }
	void OnLoopEnd(void* pBufferContext) override { }
	void OnVoiceError(void* pBufferContext, HRESULT Error) override { }
};

AudioEngine::AudioEngine()
{
	s_Instance = this;
}

VoiceCallback vcb;
void AudioEngine::Init(int numOfChannel, int numOfVoices)
{

	HRESULT hr;

	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(hr));

	hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
	assert(SUCCEEDED(hr));

	pMasterVoice->GetVoiceDetails(&masterVoiceDetails);

	// Without clamping sample rate, it was crashing 32bit 192kHz audio devices
	if (masterVoiceDetails.InputSampleRate > 48000)
		masterVoiceDetails.InputSampleRate = 48000;

	DWORD channelMask;
	pMasterVoice->GetChannelMask(&channelMask);
	hr = X3DAudioInitialize(channelMask, X3DAUDIO_SPEED_OF_SOUND, audio3D);
	assert(SUCCEEDED(hr));

	m_Voices.reserve(numOfVoices);
	m_Submix.reserve(numOfChannel);

	WAVEFORMATEXTENSIBLE wfxStereo = { 0 };
	wfxStereo.Format.wFormatTag = 1;
	wfxStereo.Format.nChannels = 2;
	wfxStereo.Format.nSamplesPerSec = 44100;
	wfxStereo.Format.nAvgBytesPerSec = 176400;
	wfxStereo.Format.nBlockAlign = 4;
	wfxStereo.Format.wBitsPerSample = 16;

	for (int i = 0; i < numOfVoices; i++)
	{
		m_Voices.emplace_back();
		m_Voices[i].id = i;
		m_ids.emplace_back(i);
		hr = pXAudio2->CreateSourceVoice(&m_Voices[i].voice, (WAVEFORMATEX*)&wfxStereo,
			0, XAUDIO2_DEFAULT_FREQ_RATIO, &vcb, nullptr);
		assert(SUCCEEDED(hr));
	}
	for (int i = 0; i < numOfChannel; i++)
	{
		m_Submix.emplace_back();
		hr = pXAudio2->CreateSubmixVoice(&m_Submix[i].submixVoice, 2, 44100, 0, 0, 0, 0);
		assert(SUCCEEDED(hr));
		m_Submix[i].SFXSend = { 0, m_Submix[i].submixVoice };
		m_Submix[i].SFXSendList = { 1, &m_Submix[i].SFXSend };
	}
}

void AudioEngine::Play2D(Sound* sound, const int& submix)
{
	if (!sound->loaded)
	{
		//Sound not loaded
		LOG_CORE_WARN("Sound not loaded!");
		return;
	}
	if (m_ids.size() == 0) { return; }

	uint32_t id = m_ids.back();
	m_ids.pop_back();

	XAUDIO2_VOICE_DETAILS details;
	m_Voices[id].voice->GetVoiceDetails(&details);
	if (details.InputSampleRate != sound->wfx.Format.nSamplesPerSec)
		m_Voices[id].voice->SetSourceSampleRate(sound->wfx.Format.nSamplesPerSec);

	sound->buffer.pContext = (void*)id;
	sound->lastVoiceId = id;
	HRESULT hr = m_Voices[id].voice->SubmitSourceBuffer(&sound->buffer);

	m_Voices[id].voice->SetOutputVoices(&m_Submix[submix].SFXSendList);
	m_Voices[id].voice->Start();
}

void AudioEngine::Play3D(Sound* sound, const int& submix, vec3f direction, float innerRadius)
{

	if (!sound->loaded)
	{
		//Sound not loaded
		LOG_CORE_WARN("Sound not loaded!");
		return;
	}

	if (m_ids.size() == 0) { return; }

	uint32_t id = m_ids.back();
	m_ids.pop_back();

	XAUDIO2_VOICE_DETAILS details;
	m_Voices[id].voice->GetVoiceDetails(&details);
	if (details.InputSampleRate != sound->wfx.Format.nSamplesPerSec)
		m_Voices[id].voice->SetSourceSampleRate(sound->wfx.Format.nSamplesPerSec);

	sound->buffer.pContext = (void*)id;
	sound->lastVoiceId = id;
	HRESULT hr = m_Voices[id].voice->SubmitSourceBuffer(&sound->buffer);
	
	float matrix[4];
	Calculate3DMatrix(sound->wfx.Format.nChannels, matrix, direction, innerRadius);
	if (sound->wfx.Format.nChannels == 2)
	{
		//I have no idea why stereo sounds are mapped incorrectly
		float f = matrix[1];
		matrix[1] = matrix[2];
		matrix[2] = f;
	}
	m_Voices[id].voice->SetOutputVoices(&m_Submix[submix].SFXSendList);
	m_Voices[id].voice->SetOutputMatrix(nullptr, sound->wfx.Format.nChannels, masterVoiceDetails.InputChannels, matrix);
	m_Voices[id].voice->Start();
}

void AudioEngine::Update3D(Sound* sound, const int& channel, vec3f direction, float innerRadius)
{
	if (!sound->loaded)
	{
		//Sound not loaded
		LOG_CORE_WARN("Sound not loaded!");
		return;
	}

	if (sound->lastVoiceId == -1)
	{
		return;
	}

	int nChannels = sound->wfx.Format.nChannels;

	float matrix[4];
	Calculate3DMatrix(nChannels, matrix, direction, innerRadius);

	if (nChannels == 2)
	{
		float f = matrix[1];
		matrix[1] = matrix[2];
		matrix[2] = f;
	}

	m_Voices[sound->lastVoiceId].voice->SetOutputMatrix(nullptr, nChannels, masterVoiceDetails.InputChannels, matrix);
}

void AudioEngine::Stop(Sound* sound)
{
	if (!sound->loaded)
	{
		//Sound not loaded
		LOG_CORE_WARN("Sound not loaded!");
		return;
	}

	if (sound->lastVoiceId == -1)
	{
		return;
	}
	m_Voices[sound->lastVoiceId].voice->Stop();
	m_Voices[sound->lastVoiceId].voice->FlushSourceBuffers();
}

void AudioEngine::Calculate3DMatrix(int channels, float* matrix, vec3f direction, float innerRadius)
{
	X3DAUDIO_LISTENER Listener = {};
	X3DAUDIO_EMITTER Emitter = {};

	X3DAUDIO_DSP_SETTINGS DSPSettings = { 0 };
	std::vector<float> channelAzimuths;

	channelAzimuths.resize(2);
	for (size_t i = 0; i < channelAzimuths.size(); ++i)
	{
		channelAzimuths[i] = X3DAUDIO_2PI * float(i) / float(channelAzimuths.size());
	}

	DSPSettings.SrcChannelCount = channels;
	DSPSettings.DstChannelCount = masterVoiceDetails.InputChannels;
	DSPSettings.pMatrixCoefficients = matrix;


	Emitter.OrientFront = { 0, 0, 0 };
	Emitter.OrientTop = { 0, 0, 0 };
	Emitter.Position = { 0, 0, 0 };
	Emitter.Velocity = { 0, 0, 0 };
	Emitter.pCone = NULL;
	Emitter.pLFECurve = NULL;
	Emitter.pLPFDirectCurve = NULL;
	Emitter.pLPFReverbCurve = NULL;
	Emitter.pVolumeCurve = NULL;
	Emitter.pReverbCurve = NULL;
	Emitter.InnerRadius = innerRadius;		//Blending between left and right when close to source
	Emitter.InnerRadiusAngle = 0.0f;
	Emitter.ChannelCount = channels;
	Emitter.ChannelRadius = 2.0f;
	Emitter.CurveDistanceScaler = 2.5f;
	Emitter.DopplerScaler = 2.0f;
	Emitter.pChannelAzimuths = channelAzimuths.data();


	Listener.OrientFront = { 1, 0, 0 };
	Listener.OrientTop = { 0, 1, 0 };
	Listener.Position = { direction.x, direction.y, direction.z };
	Listener.Velocity = { 0, 0, 0 };
	Listener.pCone = NULL;

	X3DAudioCalculate(audio3D, &Listener, &Emitter,
		X3DAUDIO_CALCULATE_MATRIX,
		&DSPSettings);
}

void AudioEngine::SetVolume(float volume, const int& channel)
{
	if (channel >= m_Submix.size())
	{
		LOG_CORE_WARN("channel %d does not exist!", channel);
		return;
	}
	m_Submix[channel].submixVoice->SetVolume(volume);
}

void AudioEngine::SetMasterVolume(float volume)
{
	pMasterVoice->SetVolume(volume);
}

void AudioEngine::Flush()
{
	for (size_t i = 0; i < m_Voices.size(); i++)
	{
		m_Voices[i].voice->Stop();
		m_Voices[i].voice->FlushSourceBuffers();
	}
}

AudioEngine::~AudioEngine()
{
	for (size_t i = 0; i < m_Voices.size(); i++)
	{
		m_Voices[i].voice->DestroyVoice();
	}
	for (int i = 0; i < m_Submix.size(); i++)
	{
		m_Submix[i].submixVoice->DestroyVoice();
	}

	pMasterVoice->DestroyVoice();
	pXAudio2->Release();
	CoUninitialize();
}

void AudioEngine::LoadData(Sound* sound, std::string filename, bool looping)
{
	std::filesystem::path path = filename;
	std::string extension = path.extension().string();

	if (extension == ".mp3")  LoadMp3(sound, filename, looping);
	if (extension == ".ogg")  LoadOgg(sound, filename, looping);
	if (extension == ".wav")  LoadWav(sound, filename, looping);
}
Sound* AudioEngine::LoadData(std::string filename, bool looping)
{
	Sound* sound = new Sound;

	std::filesystem::path path = filename;
	std::string extension = path.extension().string();
	
	if (extension == ".mp3")  LoadMp3(sound, filename, looping);
	if (extension == ".ogg")  LoadOgg(sound, filename, looping);
	if (extension == ".wav")  LoadWav(sound, filename, looping);

	return sound;
}

void AudioEngine::LoadWav(Sound* sound, std::string filename, bool looping)
{
	HRESULT hr;

	// Open the file
	HANDLE hFile = CreateFileA(
		(LPCSTR)filename.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		LOG_CORE_ERROR("audio file: %s could not be opened", filename.c_str());
		return;
	}

	SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

	DWORD dwChunkSize;
	DWORD dwChunkPosition;

	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, 'FFIR', dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

	FindChunk(hFile, ' tmf', dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &sound->wfx, dwChunkSize, dwChunkPosition);

	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, 'atad', dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	sound->buffer.AudioBytes = dwChunkSize;
	sound->buffer.pAudioData = pDataBuffer;
	sound->buffer.Flags = XAUDIO2_END_OF_STREAM;
	sound->buffer.LoopCount = looping ? XAUDIO2_LOOP_INFINITE : 0;

	sound->loaded = true;
}
void AudioEngine::LoadMp3(Sound* sound, std::string filename, bool looping)
{
	mp3dec_file_info_t info;
	int loadResult = mp3dec_load(&s_Mp3d, filename.c_str(), &info, NULL, NULL);
	if (loadResult == -3)
	{
		LOG_CORE_ERROR("audio file: %s could not be opened", filename.c_str());
		return;
	}
	uint32_t size = info.samples * sizeof(mp3d_sample_t);

	auto sampleRate = info.hz;
	auto channels = info.channels;

	sound->buffer.AudioBytes = size;
	sound->buffer.pAudioData = (BYTE*)info.buffer;
	sound->buffer.Flags = XAUDIO2_END_OF_STREAM;
	sound->buffer.LoopCount = looping ? XAUDIO2_LOOP_INFINITE : 0;

	sound->wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
	sound->wfx.Format.nChannels = channels;
	sound->wfx.Format.nSamplesPerSec = sampleRate;
	sound->wfx.Format.wBitsPerSample = sizeof(short) * 8;
	sound->wfx.Format.nBlockAlign = channels * sizeof(short);
	sound->wfx.Format.nAvgBytesPerSec = sound->wfx.Format.nSamplesPerSec * sound->wfx.Format.nBlockAlign;

	sound->loaded = true;
}
void AudioEngine::LoadOgg(Sound* sound, std::string filename, bool looping)
{
	FILE* fp;
	fopen_s(&fp, filename.c_str(), "rb");
	if (!fp) {
		LOG_CORE_ERROR("audio file: &s could not be opened", filename);
	}
	//Open sound stream. 
	OggVorbis_File vorbis;
	if (ov_open_callbacks(fp, &vorbis, NULL, 0, OV_CALLBACKS_DEFAULT) != 0) {
		LOG_CORE_ERROR("audio file: %s could not be opened", filename.c_str());
		return;
	}
	// Print sound information. 
	vorbis_info* info = ov_info(&vorbis, -1);

	uint64_t samples = ov_pcm_total(&vorbis, -1);
	uint32_t bufferSize = 2 * info->channels * samples; // 2 bytes per sample (I'm guessing...)
	//printf("Ogg file %d Hz, %d channels, %d kbit/s.\n", info->rate, info->channels, info->bitrate_nominal / 1024);

	uint8_t* oggBuffer = new uint8_t[bufferSize];
	uint8_t* bufferPtr = oggBuffer;
	int eof = 0;
	while (!eof)
	{
		int current_section;
		long length = ov_read(&vorbis, (char*)bufferPtr, 4096, 0, 2, 1, &current_section);
		bufferPtr += length;
		if (length <= 0)
		{
			eof = 1;
		}
	}

	sound->buffer.AudioBytes = bufferSize;
	sound->buffer.pAudioData = oggBuffer;
	sound->buffer.Flags = XAUDIO2_END_OF_STREAM;
	sound->buffer.LoopCount = looping ? XAUDIO2_LOOP_INFINITE : 0;

	sound->wfx.Format.wFormatTag = WAVE_FORMAT_PCM;
	sound->wfx.Format.nChannels = info->channels;
	sound->wfx.Format.nSamplesPerSec = info->rate;
	sound->wfx.Format.wBitsPerSample = sizeof(short) * 8;
	sound->wfx.Format.nBlockAlign = info->channels * sizeof(short);
	sound->wfx.Format.nAvgBytesPerSec = sound->wfx.Format.nSamplesPerSec * sound->wfx.Format.nBlockAlign;

	sound->loaded = true;

	ov_clear(&vorbis);
}

HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK)
	{
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());
			break;

		default:
			if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
				return HRESULT_FROM_WIN32(GetLastError());
		}

		dwOffset += sizeof(DWORD) * 2;

		if (dwChunkType == fourcc)
		{
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;

}

HRESULT ReadChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}