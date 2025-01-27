#include "RealEngine.h"
#include "Grenade.h"

#include "LightFlash.h"
#include "IDamageable.h"

Grenade::Grenade(float _radius, float _strength, float _falloff, float _timer) : GameObject(),
m_RADIUS(_radius),
m_FORCE(_strength),
m_FALOFF(_falloff)
{
	m_timer = _timer;
}

void Grenade::Tick(float _deltaTime)
{
	m_timer -= _deltaTime;
	if (m_timer <= 0)
	{
		Explode();
		m_engine->m_loadedScene->GetObjectManager().RemoveObject(*this);
	}
}

void Grenade::Init(real::RealEngine& _engine, real::Model& _objectModel, real::E_GENERATECOLLISIONSHAPEMODE _generateCollisionShape, float _mass, glm::vec3 _startPos, glm::vec3 _startScale, glm::quat _startRotation)
{
	m_engine = &_engine;
	GameObject::Init(_objectModel, _generateCollisionShape, _mass, _startPos, _startScale, _startRotation);
}

void Grenade::Explode()
{

	m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/boom.wav", 1.5f, true, GetPosition(), 0.0025f);


	//Spawn particles
	//Fire
	real::ParticleSystem* particles = new real::ParticleSystem(500, 1.f, glm::vec2(0.1f, 0.7f), 1, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(15.f, 50.f);
	particles->m_constantForce = glm::vec3(0, -10, 0);
	particles->m_startPositionSize = glm::vec3(1.0f);
	particles->m_position = GetPosition();
	particles->m_particleSize = 0.7f;
	particles->m_startColor = glm::vec3(1.f, 0.3f, 0.f);
	particles->m_endColor = glm::vec3(0.f, 0.f, 0.f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	//Sparks
	particles = new real::ParticleSystem(500, 1.f, glm::vec2(0.1f, 0.4f), 1, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(15.f, 50.f);
	particles->m_constantForce = glm::vec3(0, -10, 0);
	particles->m_startPositionSize = glm::vec3(1.0f);
	particles->m_position = GetPosition();
	particles->m_particleSize = 0.5f;
	particles->m_startColor = glm::vec3(1.f, 0.6f, 0.f);
	particles->m_endColor = glm::vec3(1.f, 1.f, 1.f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	//Smoke
	particles = new real::ParticleSystem(25, 0.1f, glm::vec2(1.f, 3.5f), 3, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(5.f, 15.f);
	particles->m_constantForce = glm::vec3(0, 2, 0);
	particles->m_startPositionSize = glm::vec3(0.0f);
	particles->m_position = GetPosition();
	particles->m_particleSize = 0.6f;
	particles->m_startColor = glm::vec3(0.5f, 0.5f, 0.5f);
	particles->m_endColor = glm::vec3(0.f, 0.f, 0.f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	LightFlash* flash = new LightFlash(0.2f, real::E_lightTypes::pointLight, glm::vec3(1.5f, 0.8f, 0.5f), glm::vec3(0.8f, 0.6f, 0.5f), 0.01f, 0.002f, 0.01f);
	flash->Init(*m_engine);
	flash->SetPosition(GetPosition());
	m_engine->m_loadedScene->GetObjectManager().AddObject(*flash);

	for (unsigned int i = 0; i < m_engine->m_loadedScene->GetObjectManager().GetObjectPoolSize(); i++)
	{
		GameObject* object = m_engine->m_loadedScene->GetObjectManager().GetObjects()[i];
		if (object == nullptr) continue;


		if (glm::length(GetPosition() - object->GetPosition()) < m_RADIUS)
		{

			if (IDamageable* damageable = dynamic_cast<IDamageable*>(object))
			{
				bool destroyed = damageable->TakeDamage(10.f, object->GetPosition(), glm::normalize(object->GetPosition() - GetPosition()));
				if (destroyed) continue;
			}

			if (object->m_body->getCollisionShape() == nullptr) continue;
			//Apply force at aabb to approximate a more realistic distribution of force without having to apply to all contact points
			btVector3 aabbMin, aabbMax;
			object->m_body->getAabb(aabbMin, aabbMax);
			btVector3 corners[8];
			corners[0] = btVector3(aabbMin.getX(), aabbMin.getY(), aabbMin.getZ());
			corners[1] = btVector3(aabbMax.getX(), aabbMin.getY(), aabbMin.getZ());
			corners[2] = btVector3(aabbMin.getX(), aabbMax.getY(), aabbMin.getZ());
			corners[3] = btVector3(aabbMax.getX(), aabbMax.getY(), aabbMin.getZ());
			corners[4] = btVector3(aabbMin.getX(), aabbMin.getY(), aabbMax.getZ());
			corners[5] = btVector3(aabbMax.getX(), aabbMin.getY(), aabbMax.getZ());
			corners[6] = btVector3(aabbMin.getX(), aabbMax.getY(), aabbMax.getZ());
			corners[7] = btVector3(aabbMax.getX(), aabbMax.getY(), aabbMax.getZ());

			for (int j = 0; j < 8; ++j)
			{
				glm::vec3 offset = GetPosition() - BtVecToGlmVec(corners[j]);
				float distanceSqr = glm::dot(offset, offset);

				float force = std::max(m_FORCE - m_FALOFF * distanceSqr, 0.f);
				if (force > 0.0f)
				{
					object->m_body->applyImpulse(GlmVecToBtVec(glm::normalize(offset) * -force), corners[j] - GlmVecToBtVec(object->GetPosition()));
				}
			}
		}
	}
}


