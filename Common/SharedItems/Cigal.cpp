#include "RealEngine.h"
#include "Cigal.h"

#include "Player.h"

Cigal::Cigal(const Item& baseItem) : Item(baseItem)
{
}

void Cigal::Init(real::Model* _itemModel, Player& _player, real::RealEngine& _engine)
{
	Item::Init(_itemModel);
	m_player = &_player;
	m_engine = &_engine;
}

void Cigal::Use(float actionValue)
{
	if (actionValue == 1)
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/smoke.wav");

		m_player->TakeDamage(1);
		//Spawn particles
		real::ParticleSystem* particles = new real::ParticleSystem(0.1f, glm::vec2(0.8f, 5.0f), 2.f, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(0.2f, 0.5f);
		particles->m_constantForce = glm::vec3(0.f, 0.15f, 0.f);
		particles->m_startPositionSize = glm::vec3(0.0f);
		particles->m_spawnDirection = glm::vec3(0,0,1);
		particles->m_spawnDirectionRandomness = 0.8f;
		particles->m_position = glm::vec3(0.0f, 1.5f, 0.55f);
		particles->m_followObject = m_player;
		particles->m_particleSize = 0.05f;
		particles->m_startColor = glm::vec3(0.5f, 0.5f, 0.5f);
		particles->m_endColor = glm::vec3(0.0f, 0.0f, 0.0f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);

		Remove(1);
	}else if (actionValue == -1)
	{
	}
	else
	{
		
	}
}