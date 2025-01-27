#pragma once

namespace real
{

	class Model;
	class Camera;
	class ComputeShader;
	class Scene;
	class GameObject;

	struct Particle
	{
		glm::vec3 m_position;
		float m_size;
		glm::vec3 m_velocity;
		float m_life;
		float m_lifeTime;
		float pad1;
		float pad2;
		float pad3;
	};

	class ParticleSystem
	{
	public:
		ParticleSystem(float spawnRate = 0.1f, glm::vec2 minMaxLifeTime = glm::vec2(0.5f, 1.f), float duration = -1.f, bool rotateTowardsCamera = false);
		ParticleSystem(unsigned int particlesPerBurst, float spawnRate = 0.1f, glm::vec2 minMaxLifeTime = glm::vec2(1.f,2.f), int burstsUntillDeath = -1, bool rotateTowardsCamera = false);
		~ParticleSystem();
		void Init(ComputeShader& computeShader, Model& particleModel, Camera& mainCamera, Scene& scene);
		void Tick(float deltaTime);
		void Draw();

		glm::vec2 m_spawnVelocityMinMax{ glm::vec2(1.f,1.f) };
		glm::vec3 m_constantForce{ glm::vec3(0.f,-10.f,0.f) };
		glm::vec3 m_startPositionSize{ glm::vec3(1.f) }; // Possible offset in an elips for the particles to spawn in
		glm::vec3 m_spawnDirection{ glm::vec3(0.f) };
		float m_spawnDirectionRandomness{ 1.f };
		glm::vec3 m_startColor{ glm::vec3(1.f,0.f,0.f) };
		glm::vec3 m_endColor{ glm::vec3(0.f,0.f,0.f) };
		glm::vec3 m_position{ glm::vec3(0.f,5.f,0.f) };
		GameObject* m_followObject{ nullptr }; // Object to follow position is used as offset instead
		float m_particleSize{ 1.f };
	private:
		Scene* m_scene{ nullptr };
		ComputeShader* m_particleShader{ nullptr };
		Model* m_particleModel{ nullptr };
		Camera* m_mainCamera{ nullptr };
		const bool m_ROTATE_TO_CAMERA{ true };
		const int m_PARTICLE_AMOUNT{ 100 };
		const glm::vec2 m_PARTICLE_LIFETIME{ glm::vec2(0.5f,1.f) };
		const float m_SPAWN_RATE{ 0.1f };
		const bool m_BURST{ false };
		const int m_BURST_AMOUNT{ 0 };
		int m_burstsUntillDeath = -1;
		float m_stopTimer = -1;
		float m_deathTimer = -1;
		float m_spawnTimer{ 0 };
		unsigned int m_dataTexture{ 0 };
		unsigned int m_atomicCounter{ 0 };
	};
}