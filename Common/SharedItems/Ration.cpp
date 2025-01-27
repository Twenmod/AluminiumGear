#include "RealEngine.h"
#include "Ration.h"

#include "Player.h"

Ration::Ration(const Item& baseItem) : Item(baseItem)
{
}

void Ration::Init(real::Model* _itemModel, Player& _player, real::RealEngine& _engine)
{
	Item::Init(_itemModel);
	m_player = &_player;
	m_engine = &_engine;
}

void Ration::Use(float actionValue)
{
	if (actionValue == 1)
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/heal.wav");

		m_player->Heal(30);
		//Spawn particles
		real::ParticleSystem* particles = new real::ParticleSystem(20,1.f, glm::vec2(0.8f, 1.0f), 1, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(1.f, 15.f);
		particles->m_constantForce = glm::vec3(0.f, -2.f, 0.f);
		particles->m_startPositionSize = glm::vec3(0.5f,3.f,0.5f);
		particles->m_position = glm::vec3(0.f, 0.f, 0.f);
		particles->m_followObject = m_player;
		particles->m_particleSize = 0.15f;
		particles->m_startColor = glm::vec3(0.3f, 1.f, 0.3f);
		particles->m_endColor = glm::vec3(0.6f, 1.0f, 0.6f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);

		Remove(1);
	}else if (actionValue == -1)
	{
	}
	else
	{
		
	}
}