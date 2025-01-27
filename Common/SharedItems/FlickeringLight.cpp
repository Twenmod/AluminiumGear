#include "RealEngine.h"
#include "FlickeringLight.h"

FlickeringLight::FlickeringLight(float _flickerBaseStrength, glm::vec2 _flickerInterval, glm::vec2 _flickerDuration, real::E_lightTypes _type, glm::vec3 _color, glm::vec3 _specular, float _constant, float _linear, float _quadratic, float _minAngle, float _maxAngle) : 
	Light(_type,_color, _specular, _constant, _linear, _quadratic, _minAngle, _maxAngle),
	m_FLICKERING_BASE_STRENGTH(_flickerBaseStrength),
	m_FLICKERING_INTERVAL(_flickerInterval),
	m_FLICKER_DURATION(_flickerDuration)
{
}


void FlickeringLight::Init(real::RealEngine& _engine)
{
	m_engine = &_engine;
}

void FlickeringLight::Tick(float _deltaTime)
{
	if (m_pulsing < 0) m_strength = glm::mix(m_strength, 1.f, 1 - exp2(-_deltaTime / (m_FLICKER_SMOOTH_HALFTIME)));
	else  m_strength = glm::mix(m_strength, m_FLICKERING_BASE_STRENGTH, 1 - exp2(-_deltaTime / (m_FLICKER_SMOOTH_HALFTIME * 0.25f)));

	m_pulseTimer -= _deltaTime;
	m_pulsing -= _deltaTime;
	if (m_pulseTimer <= 0)
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/spark.wav",0.5f,true,GetPosition(),0.05f);


		m_pulseTimer = std::max((rand() / (float)RAND_MAX) * m_FLICKERING_INTERVAL.y, m_FLICKERING_INTERVAL.x);
		m_currentFlickedDuration = std::max((rand() / (float)RAND_MAX) * m_FLICKER_DURATION.y, m_FLICKER_DURATION.x);
		m_pulsing = m_currentFlickedDuration;
		//Spawn particles
		real::ParticleSystem* particles = new real::ParticleSystem(5, 1.f, glm::vec2(0.1f, 1.2f), 1, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(1.f, 10.f);
		particles->m_constantForce = glm::vec3(0.f, -20.f, 0.f);
		particles->m_startPositionSize = glm::vec3(0.05f);
		particles->m_position = GetPosition();
		particles->m_particleSize = 0.05f;
		particles->m_startColor = glm::vec3(1.0f, 0.9f, 0.2f);
		particles->m_endColor = glm::vec3(0.5f, 0.5f, 0.5f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);
	}
}
