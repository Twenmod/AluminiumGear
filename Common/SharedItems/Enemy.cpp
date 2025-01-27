#include "precomp.h"
#include "RealEngine.h"
#include "Enemy.h"
#include "GibbedGameObject.h"
#include "ParticleSystem.h"
#include "LightFlash.h"

Enemy::Enemy() : real::AnimatedGameObject()
{

}

Enemy::~Enemy()
{
	delete m_body->getCollisionShape();
}


void Enemy::Init(real::NavMesh& _navMesh, real::RealEngine& _engine, real::Model& _model, const char* _startAnimationPath, real::Model& _gibModel, float _mass, glm::vec3 _startPos, glm::vec3 _startScale, glm::quat _startRotation)
{
	m_engine = &_engine;
	m_navMesh = &_navMesh;
	m_physicsWorld = _engine.m_btDynamicsWorld;
	m_objectManager = &_engine.m_loadedScene->GetObjectManager();
	m_textureManager = &_engine.m_textureManager;
	real::AnimatedGameObject::Init(_model, _startAnimationPath, real::E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE, _mass, _startPos, _startScale, _startRotation);
	LoadAnimation("../Common/Assets/Models/Enemy/WalkModel.dae", E_ENEMYANIMATIONS::Walking);
	LoadAnimation("../Common/Assets/Models/Enemy/Aim.dae", E_ENEMYANIMATIONS::Aim, false);
	LoadAnimation("../Common/Assets/Models/Enemy/AimWalk.dae", E_ENEMYANIMATIONS::AimWalk);
	LoadAnimation("../Common/Assets/Models/Enemy/Die.dae", E_ENEMYANIMATIONS::Die, false);

	m_animator->SetPlayBackSpeed(1);
	m_body->setCollisionShape(new btCapsuleShape(25, 120));
	m_body->setFriction(0);
	m_modelOffset = glm::vec3(0, -2.2, 0);
	m_gibModel = &_gibModel;
}

void Enemy::Tick(float _deltaTime)
{
	real::AnimatedGameObject::Tick(_deltaTime);
	if (m_health <= 0) return;
	if (m_target != nullptr)
	{
		m_seesPlayer = TargetLineOfSight();

		if (m_triggerTime > 0)
		{
			TriggerBehaviour(_deltaTime);
		}
		else
		{
			CalmBehaviour(_deltaTime);
		}
		if (m_currentPath.size() > 0)
		{
			m_navMesh->DrawPath(m_currentPath);
		}
	}

}

void Enemy::TriggerBehaviour(float _deltaTime)
{
	if (!TargetLineOfSight())
	{

		m_currentPath = m_navMesh->PlotPath(GetPosition(), m_target->GetPosition());
		m_navMesh->DrawPath(m_currentPath);
		if (m_currentPath.size() > 2)
		{
			glm::vec3 targetDir = (m_currentPath[1]->position + m_currentPath[2]->position) / 2.f - GetPosition();
			targetDir = glm::normalize(targetDir);
			m_body->setLinearVelocity(GlmVecToBtVec(targetDir * m_FOLLOWINGWALKSPEED));
			m_body->setActivationState(1);
			targetDir.y = 0;
			glm::quat targetRot = glm::quatLookAt(glm::normalize(-targetDir), glm::vec3(0, 1, 0));
			SetRotation(glm::slerp(GetRotation(), targetRot, 1 - exp2(-_deltaTime / m_ROTATIONLERPTIME)));
		}
		m_triggerTime -= _deltaTime;
		float velocityMag = m_body->getLinearVelocity().length();
		m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::AimWalk]);
		m_animator->SetPlayBackSpeed(velocityMag * m_WALKANIMSPEED);
	}
	else
	{
		glm::vec3 targetDir = (m_target->GetPosition() - GetPosition());

		if (glm::length(targetDir) > m_COMBATSTOPRANGE)
		{
			//Move closer
			m_currentPath = m_navMesh->PlotPath(GetPosition(), m_target->GetPosition());
			m_navMesh->DrawPath(m_currentPath);
			if (m_currentPath.size() > 2)
			{
				glm::vec3 walkTargetDir = (m_currentPath[1]->position + m_currentPath[2]->position) / 2.f - GetPosition();
				walkTargetDir = glm::normalize(walkTargetDir);
				m_body->setLinearVelocity(GlmVecToBtVec(walkTargetDir * m_FOLLOWINGWALKSPEED*0.5f));
				m_body->setActivationState(1);
			}
			m_shootTimer -= _deltaTime / 2; // Shoot slower
			float velocityMag = m_body->getLinearVelocity().length();
			m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::AimWalk]);
			m_animator->SetPlayBackSpeed(velocityMag * m_WALKANIMSPEED);
		}
		else
		{
			m_shootTimer -= _deltaTime*1.5f; // Shoot Faster
			m_body->setLinearVelocity(btVector3(0, m_body->getLinearVelocity().getY(), 0));
			m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::Aim], 0.25f, 1000);
			m_animator->SetPlayBackSpeed(1);
		}

		//Shoot
		m_triggerTime = m_TRIGGERINGDURATION;

		targetDir.y = 0;
		glm::quat targetRot = glm::quatLookAt(glm::normalize(-targetDir), glm::vec3(0, 1, 0));
		SetRotation(glm::slerp(GetRotation(), targetRot, 1 - exp2(-_deltaTime / m_ROTATIONLERPTIME)));



		if (m_shootTimer <= 0)
		{
			m_shootTimer = m_SHOOT_INTERVAL;
			Shoot();
		}


	}

}

void Enemy::CalmBehaviour(float _deltaTime)
{
	m_body->setLinearVelocity(btVector3(0, m_body->getLinearVelocity().getY(), 0));
	if (m_seesPlayer) // goto trigger mode
	{
		m_currentPath = m_navMesh->PlotPath(GetPosition(), m_target->GetPosition());
		m_triggerTime = m_TRIGGERINGDURATION;
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/alert.wav", 1.0f, true, GetPosition(), 0.0f);
		m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::Aim], 0.25f, 0);
	}

	if (m_patrolPath.size() > 0)
	{
		//Patrolling
		if (glm::length(GetPosition() - m_patrolPath[m_currentPatrolPosition]) < m_POSITIONREACHEDDISTANCE)
		{
			//Reached point
			m_currentPatrolPosition = (m_currentPatrolPosition + 1) % m_patrolPath.size();
			m_currentPath = m_navMesh->PlotPath(GetPosition(), m_patrolPath[m_currentPatrolPosition]);
		}
		if (m_currentPath.size() > 0)
		{
			if (m_currentPath.size() > 1)
			{
				if (glm::length(GetPosition() - m_currentPath[1]->position) < m_POSITIONREACHEDDISTANCE)
				{
					m_currentPath.erase(m_currentPath.begin());
				}
			}
			glm::vec3 targetDir;
			if (m_currentPath.size() > 2)
			{
				targetDir = (m_currentPath[1]->position + m_currentPath[2]->position) / 2.f - GetPosition();
				targetDir = glm::normalize(targetDir);

			}
			else
			{
				targetDir = m_currentPath[m_currentPath.size() - 1]->position - GetPosition();
				targetDir = glm::normalize(targetDir);
			}
			m_body->setLinearVelocity(GlmVecToBtVec(targetDir * m_PATROLWALKSPEED));
			m_body->setActivationState(1);
			targetDir.y = 0;
			glm::quat targetRot = glm::quatLookAt(glm::normalize(-targetDir), glm::vec3(0, 1, 0));
			SetRotation(glm::slerp(GetRotation(), targetRot, 1 - exp2(-_deltaTime / m_ROTATIONLERPTIME)));
		}
	}

	//Animation
	float velocityMag = m_body->getLinearVelocity().length();
	if (velocityMag <= 0.1f)
	{
		m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::Idle]);
		m_animator->SetPlayBackSpeed(1);
	}
	else
	{
		m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::Walking]);
		m_animator->SetPlayBackSpeed(velocityMag * m_WALKANIMSPEED);
	}
}

void Enemy::Shoot()
{
	btVector3 from = GlmVecToBtVec(GetPosition() + glm::vec3(0, 0.2f, 0));
	glm::vec3 dir = normalize((m_target->GetPosition() + glm::vec3(0, 1.7f, 0)) - (GetPosition() + glm::vec3(0, 1.7f, 0)));
	//Inaccuracy
	dir = normalize(dir + (glm::vec3(rand() / (float)RAND_MAX, rand() / (float)RAND_MAX, rand() / (float)RAND_MAX) - glm::vec3(0.5)) * m_INACCURACY);

	glm::vec3 lookDir = GetRotation() * glm::vec3(0, 0, 1);
	if (1 - glm::dot(dir, lookDir) > acos(glm::radians(m_COMBATVIEWANGLE))) return;

	m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/enemyShoot.wav", 1.f, true, GetPosition());

	//Spawn particles
	real::ParticleSystem* particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.05f, 0.15f), 1, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(5.f, 15.f);
	particles->m_constantForce = glm::vec3(0);
	particles->m_startPositionSize = glm::vec3(0.0f);
	particles->m_position = GetPosition() + GetRotation() * glm::vec3(-0.35f, 1.6f, 2.4f);
	particles->m_spawnDirection = dir;
	particles->m_spawnDirectionRandomness = 0.05f;
	particles->m_particleSize = 0.1f;
	particles->m_startColor = glm::vec3(1.f, 0.3f, 0.f);
	particles->m_endColor = glm::vec3(1.f, 0.5f, 0.3f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	LightFlash* muzzleFlash = new LightFlash(0.06f, real::E_lightTypes::pointLight, glm::vec3(1.f, 0.8f, 0.5f), glm::vec3(0.8f, 0.6f, 0.5f), 0.05f, 0.01f, 0.05f);
	muzzleFlash->Init(*m_engine);
	muzzleFlash->SetPosition(GetPosition() + GetRotation() * glm::vec3(-0.35f, 1.6f, 2.4f));
	m_engine->m_loadedScene->GetObjectManager().AddObject(*muzzleFlash);


	//Bullet
	particles = new real::ParticleSystem(1, 1.f, glm::vec2(1.f), 1, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particle"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(150.f);
	particles->m_constantForce = glm::vec3(0.f);
	particles->m_startPositionSize = glm::vec3(0.0f);
	particles->m_spawnDirection = glm::normalize(dir);
	particles->m_spawnDirectionRandomness = 0.0f;
	particles->m_position = GetPosition() + GetRotation() * glm::vec3(-0.35f, 1.6f, 2.4f);
	particles->m_particleSize = 0.15f;
	particles->m_startColor = glm::vec3(0.5f, 0.5f, 0.5f);
	particles->m_endColor = glm::vec3(0.5f, 0.5f, 0.5f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	btVector3 to = from + GlmVecToBtVec(dir * m_SHOOT_RAY_RANGE);
	btCollisionWorld::ClosestRayResultCallback hit(from, to);
	hit.m_collisionFilterMask = BTGROUP_ALL & ~BTGROUP_ENEMY; // Dont collide with enemies
	m_physicsWorld->rayTest(from, to, hit);
	if (hit.hasHit())
	{

		glm::vec3 hitPoint = BtVecToGlmVec(from + (to - from) * hit.m_closestHitFraction);

		LightFlash* hitFlash = new LightFlash(0.1f, real::E_lightTypes::pointLight, glm::vec3(1.f, 0.8f, 0.5f), glm::vec3(0.8f, 0.6f, 0.5f), 0.1f, 0.04f, 0.15f);
		hitFlash->Init(*m_engine);
		hitFlash->SetPosition(hitPoint);
		m_engine->m_loadedScene->GetObjectManager().AddObject(*hitFlash);

		particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.2f, 0.8f), 1, true);
		particles->m_startColor = glm::vec3(1.0f, 0.9f, 0.4f);
		if (IDamageable* hitObj = dynamic_cast<IDamageable*>(static_cast<real::GameObject*>(hit.m_collisionObject->getUserPointer())))
		{
			bool died = hitObj->TakeDamage(m_BULLET_DAMAGE, hitPoint, dir);
			if (died)
			{
				m_triggerTime = 0;
				m_target = nullptr;
			}
			particles->m_startColor = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		//Spawn particles
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(1.f, 15.f);
		particles->m_constantForce = glm::vec3(0.f, -10.f, 0.f);
		particles->m_startPositionSize = glm::vec3(0.05f);
		particles->m_spawnDirection = BtVecToGlmVec(hit.m_hitNormalWorld);
		particles->m_spawnDirectionRandomness = 0.4f;
		particles->m_position = hitPoint;
		particles->m_particleSize = 0.05f;
		particles->m_endColor = glm::vec3(0.5f, 0.5f, 0.5f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);

		//Spawn particles
		particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.2f, 0.8f), 1, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(1.f, 15.f);
		particles->m_constantForce = glm::vec3(0.f, -10.f, 0.f);
		particles->m_startPositionSize = glm::vec3(0.0f);
		particles->m_spawnDirection = BtVecToGlmVec(hit.m_hitNormalWorld);
		particles->m_spawnDirectionRandomness = 0.7f;
		particles->m_position = hitPoint;
		particles->m_particleSize = 0.02f;
		particles->m_startColor = glm::vec3(1.0f, 1.0f, 1.0f);
		particles->m_endColor = glm::vec3(0.5f, 0.5f, 0.5f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);
	}
}



bool Enemy::TargetLineOfSight()
{
	btVector3 from = GlmVecToBtVec(GetPosition() + glm::vec3(0, 0.2f, 0));
	glm::vec3 dir = (m_target->GetPosition() + glm::vec3(0, 0.2f, 0)) - (GetPosition() + glm::vec3(0, 0.2f, 0));

	float maxRange = m_triggerTime > 0 ? m_COMBATLOOKRANGE : m_LOOKRANGE;
	if (glm::length(dir) > maxRange) return false;
	glm::vec3 lookDir = GetRotation() * glm::vec3(0, 0, 1);
	float fov;
	if (m_triggerTime > 0) fov = m_COMBATVIEWANGLE;
	else fov = m_PATROLVIEWANGLE;
	if (1 - glm::dot(glm::normalize(dir), lookDir) > acos(glm::radians(fov))) return false;
	btVector3 to = from + GlmVecToBtVec(dir);
	btCollisionWorld::ClosestRayResultCallback hit(from, to);
	m_physicsWorld->rayTest(from, to, hit);
	//m_physicsWorld->getDebugDrawer()->drawLine(from, to, btVector3(0, 0, 1));
	if (hit.hasHit())
	{
		if (static_cast<real::GameObject*>(hit.m_collisionObject->getUserPointer()) == m_target)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}

}

bool Enemy::TakeDamage(float _damage, glm::vec3 _hitLocation, glm::vec3 _hitDirection)
{

	m_health -= _damage;
	if (m_health <= 0)
	{
		m_animator->PlayAnimation(m_animations[E_ENEMYANIMATIONS::Die]); // No longer really used but still there ig
		m_physicsWorld->removeRigidBody(m_body);

		//Explode in gibs
		real::GibbedObjectsSpawner* gibSpawner = new real::GibbedObjectsSpawner();
		gibSpawner->Init(*m_gibModel, *m_textureManager, 10.f, GetPosition(), GetRotation());
		gibSpawner->AddToDynamicsWorld(*m_physicsWorld);
		gibSpawner->AddToScene(*m_objectManager);
		m_engine->m_loadedScene->GetObjectManager().AddGibObjects(*gibSpawner);


		//Add explosion force
		std::vector<real::GameObject*> gibs = gibSpawner->GetObjects();
		glm::vec3 center = _hitLocation;
		const float baseForce = 250.f;
		const float falloff = 1000.f;
		for (real::GameObject* gib : gibs)
		{
			gib->m_body->getBroadphaseProxy()->m_collisionFilterGroup = BTGROUP_GIBS;
			gib->m_body->getBroadphaseProxy()->m_collisionFilterMask = BTGROUP_ALL; // Collide with everything except gibs

			//Apply force at aabb to approximate a more realistic distribution of force without having to apply to all contact points
			btVector3 aabbMin, aabbMax;
			gib->m_body->getAabb(aabbMin, aabbMax);
			btVector3 corners[8];
			corners[0] = btVector3(aabbMin.getX(), aabbMin.getY(), aabbMin.getZ());
			corners[1] = btVector3(aabbMax.getX(), aabbMin.getY(), aabbMin.getZ());
			corners[2] = btVector3(aabbMin.getX(), aabbMax.getY(), aabbMin.getZ());
			corners[3] = btVector3(aabbMax.getX(), aabbMax.getY(), aabbMin.getZ());
			corners[4] = btVector3(aabbMin.getX(), aabbMin.getY(), aabbMax.getZ());
			corners[5] = btVector3(aabbMax.getX(), aabbMin.getY(), aabbMax.getZ());
			corners[6] = btVector3(aabbMin.getX(), aabbMax.getY(), aabbMax.getZ());
			corners[7] = btVector3(aabbMax.getX(), aabbMax.getY(), aabbMax.getZ());

			for (int i = 0; i < 8; ++i)
			{
				glm::vec3 offset = gib->GetPosition() - BtVecToGlmVec(corners[i]);
				float distanceSqr = glm::dot(offset, offset);

				float force = std::max(baseForce - falloff * distanceSqr, 0.f);
				if (force > 0.0f)
				{
					gib->m_body->applyImpulse(GlmVecToBtVec(_hitDirection * force), corners[i] - GlmVecToBtVec(gib->GetPosition()));
				}
			}
		}

		//Spawn particles
		real::ParticleSystem* particles = new real::ParticleSystem(150, 0.001f, glm::vec2(1.f, 3.f), 3, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particle"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(1.f, 3.f);
		particles->m_startPositionSize = glm::vec3(1.f, 3.f, 1.f);
		particles->m_position = GetPosition();
		particles->m_particleSize = 0.3f;
		m_engine->m_loadedScene->AddParticleSystem(*particles);

		m_objectManager->RemoveObject(*this);
		return true;
	}
	return false;
}