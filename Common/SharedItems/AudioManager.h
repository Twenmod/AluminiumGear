#pragma once

struct ma_engine;
struct ma_sound;

namespace real
{
	class Camera;

	class AudioManager
	{
	public:
		AudioManager();
		~AudioManager();
		void Tick();
		void SetListener(Camera& _listener) { m_listener = &_listener; }
		void PlaySoundFile(const char* soundFile, float _baseVolume = 1.f, bool _positional = false, glm::vec3 _position = glm::vec3(0), float falloff = 0.005f);
		void PlayMusic(const char* soundFile, float baseVolume = 1.f);
		void StopMusic();
	private:
		ma_engine* engine;
		Camera* m_listener{nullptr};
		std::vector<ma_sound*> m_soundsPlaying;
		ma_sound* m_currentMusic{nullptr};
	};
}