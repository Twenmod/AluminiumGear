#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h" // Before precomp to fix define warning
#include "precomp.h"
#include "AudioManager.h"

#include "Camera.h"

real::AudioManager::AudioManager()
{
	ma_result result;

	engine = new ma_engine();

	result = ma_engine_init(NULL, engine);
	if (result != MA_SUCCESS)
	{
		printf("ERROR::MINIAUDIO Audio Engine failed to start\n");
	}


}

real::AudioManager::~AudioManager()
{
	for (int i = 0; i < m_soundsPlaying.size(); i++)
	{
		// Cleanup
		ma_sound_uninit(m_soundsPlaying[i]);
		delete m_soundsPlaying[i];
	}
	if (m_currentMusic != nullptr)
	{
		ma_sound_uninit(m_currentMusic);
		delete m_currentMusic;
	}
	ma_engine_uninit(engine);
	delete engine;
}

void real::AudioManager::Tick()
{

	if (m_currentMusic != nullptr)
	{
		if (!ma_sound_is_playing(m_currentMusic))
		{
			ma_sound_start(m_currentMusic);
		}
	}

	for (int i = 0; i < m_soundsPlaying.size(); i++)
	{
		if (!ma_sound_is_playing(m_soundsPlaying[i]))
		{
			// Cleanup
			ma_sound_uninit(m_soundsPlaying[i]);
			delete m_soundsPlaying[i];
			m_soundsPlaying.erase(m_soundsPlaying.begin() + i);
			i--;
		}
	}
}

void real::AudioManager::PlaySoundFile(const char* soundFile, float _baseVolume, bool _positional, glm::vec3 _position, float _falloff)
{
	ma_result result;
	ma_sound* sound = new ma_sound;

	result = ma_sound_init_from_file(engine, soundFile, MA_SOUND_FLAG_STREAM, NULL, NULL, sound);
	if (result != MA_SUCCESS)
	{
		printf("Failed to load sound: %s\n", soundFile);
		delete sound;
		return;
	}

	float attentuation = _baseVolume;

	if (m_listener != nullptr && _positional)
	{
		glm::vec3 offset = m_listener->GetPosition() - _position;
		float dist2 = glm::dot(offset, offset);

		float divider = _falloff * (dist2 + 1.f);
		if (divider < 1.f) divider = 1.f;
		attentuation /= divider;

		offset = glm::normalize(offset);

		float pan = -glm::dot(m_listener->GetRight(), offset);
		ma_sound_set_pan(sound, pan);
	}

	ma_sound_set_volume(sound, attentuation);


	// Play the sound
	ma_sound_start(sound);
	m_soundsPlaying.push_back(sound);
}

void real::AudioManager::PlayMusic(const char* soundFile, float baseVolume)
{
	if (m_currentMusic != nullptr)
	{
		ma_sound_uninit(m_currentMusic);
		delete m_currentMusic;
	}

	ma_result result;
	ma_sound* sound = new ma_sound;

	result = ma_sound_init_from_file(engine, soundFile, MA_SOUND_FLAG_STREAM, NULL, NULL, sound);
	if (result != MA_SUCCESS)
	{
		printf("Failed to load sound: %s\n", soundFile);
		delete sound;
		return;
	}

	ma_sound_set_volume(sound, baseVolume);

	// Play the sound
	ma_sound_start(sound);
	m_currentMusic = sound;
}

void real::AudioManager::StopMusic()
{
	if (m_currentMusic != nullptr)
	{
		ma_sound_uninit(m_currentMusic);
		delete m_currentMusic;
		m_currentMusic = nullptr;
	}
}
