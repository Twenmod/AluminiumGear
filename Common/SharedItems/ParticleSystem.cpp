#include "precomp.h"
#include "ParticleSystem.h"
#include "ComputeShader.h"
#include "Model.hpp"
#include "Camera.h"
#include "Scene.h"
#include "GameObject.h"

real::ParticleSystem::ParticleSystem(float _spawnRate, glm::vec2 _lifeTime, float _duration, bool _rotateTowardsCamera) :
	m_ROTATE_TO_CAMERA(_rotateTowardsCamera),
	m_PARTICLE_LIFETIME(_lifeTime),
	m_SPAWN_RATE(_spawnRate),
	m_PARTICLE_AMOUNT(std::min(
		static_cast<int>(ceil(_lifeTime.y / _spawnRate) * 2.f)
		, GL_MAX_TEXTURE_SIZE
	)),
	m_BURST(false)
{
	m_stopTimer = _duration;
	m_spawnTimer = m_SPAWN_RATE;
}

real::ParticleSystem::ParticleSystem(unsigned int _particlesPerBurst, float _spawnRate, glm::vec2 _lifeTime, int _burstsUntillDeath, bool _rotateTowardsCamera) :
	m_ROTATE_TO_CAMERA(_rotateTowardsCamera),
	m_PARTICLE_LIFETIME(_lifeTime),
	m_SPAWN_RATE(_spawnRate),
	m_PARTICLE_AMOUNT(std::min(
		std::min(
			static_cast<int>(ceil(_lifeTime.y* _particlesPerBurst) / _spawnRate * 2.f)
			, static_cast<int>(_particlesPerBurst* _burstsUntillDeath)
		)
		, GL_MAX_TEXTURE_SIZE
	)),
	m_BURST_AMOUNT(_particlesPerBurst),
	m_BURST(true)
{
	m_burstsUntillDeath = _burstsUntillDeath;
	m_spawnTimer = m_SPAWN_RATE;
}

real::ParticleSystem::~ParticleSystem()
{
	glDeleteBuffers(1, &m_atomicCounter);
	glDeleteTextures(1, &m_dataTexture);
}

//HOW THE DATA IS STORED IN THE texture
/*
y coords  value
0         posX
1         posY
2         posZ
3         velX
4         velY
5         velZ
6         life
7         lifetime
*/


void real::ParticleSystem::Init(ComputeShader& _computeShader, Model& _particleModel, Camera& _mainCamera, Scene& _scene)
{
	m_particleShader = &_computeShader;
	m_particleModel = &_particleModel;
	m_mainCamera = &_mainCamera;
	m_scene = &_scene;

	//Create particle array
	std::vector<float> initialParticleData(m_PARTICLE_AMOUNT * 8);
	for (int i = 0; i < (int)m_PARTICLE_AMOUNT; i++)
	{
		initialParticleData[i + m_PARTICLE_AMOUNT * 0] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 1] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 2] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 3] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 4] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 5] = 0.f;
		initialParticleData[i + m_PARTICLE_AMOUNT * 6] = 0.f; // Life
		initialParticleData[i + m_PARTICLE_AMOUNT * 7] = 0.f; // Lifetime
	}

	//Create data texture

	glGenTextures(1, &m_dataTexture);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);

	glTexStorage2D(GL_TEXTURE_2D, 1, GL_R32F, m_PARTICLE_AMOUNT, 8); // 8 since our particle data requires 8 chunks

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_PARTICLE_AMOUNT, 8, GL_RED, GL_FLOAT, initialParticleData.data());
	glBindTexture(GL_TEXTURE_2D, 0);


	//Create atomic counter
	glGenBuffers(1, &m_atomicCounter);
	// Bind and allocate memory for the atomic counter
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_atomicCounter);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), NULL, GL_DYNAMIC_DRAW);
	// Reset the counter to 0
	GLuint zero = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, m_atomicCounter);

}

void real::ParticleSystem::Tick(float _deltaTime)
{
	if (m_stopTimer > 0) m_stopTimer -= _deltaTime;
	else if (m_stopTimer != -1.f && m_deathTimer == -1.f) m_deathTimer = m_PARTICLE_LIFETIME.y;
	if (m_burstsUntillDeath == 0 && m_deathTimer == -1.f) m_deathTimer = m_PARTICLE_LIFETIME.y;

	if (m_deathTimer > 0) m_deathTimer -= _deltaTime;

	if (m_deathTimer != -1.f && m_deathTimer < 0)
	{
		m_scene->DeleteParticleSystem(*this);
		return;
	}

	//Setup data
	m_particleShader->use();
	m_particleShader->setFloat("deltaTime", _deltaTime);
	if (m_followObject != nullptr )	m_particleShader->setVec3("startPosition", m_followObject->GetPosition()+m_followObject->GetRotation()*m_position);
	else m_particleShader->setVec3("startPosition", m_position);
	m_particleShader->setVec3("startPosSize", m_startPositionSize);
	m_particleShader->setVec3("gravity", m_constantForce);
	m_particleShader->setInt("maxParticles", m_PARTICLE_AMOUNT);
	if (m_BURST) m_particleShader->setInt("particlesPerFrame", m_BURST_AMOUNT);
	else m_particleShader->setInt("particlesPerFrame", static_cast<int>(ceil(_deltaTime / m_SPAWN_RATE)));

	if (m_followObject != nullptr) m_particleShader->setVec3("spawnDirection", m_followObject->GetRotation() * m_spawnDirection);
	else m_particleShader->setVec3("spawnDirection", m_spawnDirection);
	m_particleShader->setFloat ("spawnDirectionRandomness", m_spawnDirectionRandomness);
	m_particleShader->setVec2("spawnVelocity", m_spawnVelocityMinMax);
	m_particleShader->setVec2("particleLifeTime", m_PARTICLE_LIFETIME);
	m_particleShader->setInt("seed", static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count()));

	glBindImageTexture(0, m_dataTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);
	
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, m_atomicCounter);

	m_spawnTimer += _deltaTime;
	if (m_spawnTimer > m_SPAWN_RATE && (m_stopTimer == -1.f || m_stopTimer > 0) && (m_burstsUntillDeath == -1 || m_burstsUntillDeath > 0) && (m_deathTimer < 0))
	{
		if (m_burstsUntillDeath > 0) m_burstsUntillDeath--;
		m_spawnTimer = 0;
		//Reset the atomic counter
		GLuint zero = 0;
		glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
	}
	//Compute
	glDispatchCompute((unsigned int)m_PARTICLE_AMOUNT, 1, 1);
	// make sure writing finished
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);
}

void real::ParticleSystem::Draw()
{

	const Shader& shader = m_particleModel->GetShader();
	shader.use();
	shader.setMat4("Model", glm::mat4(1));
	shader.setMat3("NormalMatrix", glm::mat3(1));
	shader.setMat4("VP", m_mainCamera->GetViewProjection());

	shader.setInt("particleDataTexture", DATATEXTUREPOSITION);

	shader.setVec3("particleStartColor", m_startColor);
	shader.setVec3("particleEndColor", m_endColor);
	shader.setFloat("particleSize", m_particleSize);

	glActiveTexture(GL_TEXTURE0 + DATATEXTUREPOSITION);
	glBindTexture(GL_TEXTURE_2D, m_dataTexture);

	m_particleModel->DrawInstanced(m_PARTICLE_AMOUNT, std::vector<real::ReflectionProbe*>(), false);
}
