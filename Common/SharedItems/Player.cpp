#include "RealEngine.h"
#include "Player.h"

#include "Animator.h"
#include "IDamageable.h"

#include "ParticleSystem.h"

#include "LightFlash.h"

Player::Player()
{

}

Player::~Player()
{
	m_inputManager->UnSubscribe(this);
	delete m_body->getCollisionShape();
}

void Player::Init(real::InputManager& _inputManager, real::Model& _model, real::RealEngine& _engine)
{
	m_engine = &_engine;
	m_physicsWorld = m_engine->m_btDynamicsWorld;
	real::AnimatedGameObject::Init(_model, "../Common/Assets/Models/Player/ModelIdle.dae", real::E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE, 80);
	LoadAnimation("../Common/Assets/Models/Player/Walking.dae", real::E_PLAYERANIMATIONS::Walk);
	LoadAnimation("../Common/Assets/Models/Player/Run.dae", real::E_PLAYERANIMATIONS::Run);
	LoadAnimation("../Common/Assets/Models/Player/Aiming.dae", real::E_PLAYERANIMATIONS::Aim, false);
	LoadAnimation("../Common/Assets/Models/Player/Aimwalk.dae", real::E_PLAYERANIMATIONS::AimWalk);

	m_shape = new btCylinderShape(btVector3(5, 16, 5));

	m_body->setCollisionShape(m_shape);
	m_body->setRestitution(0.1f);
	m_body->setFriction(1.f);
	m_modelOffset = glm::vec3(0, -2.2f, 0);

	m_inputManager = &_inputManager;
	m_inputManager->Subscribe(this);
	m_thirdPersonCam.Init(*m_inputManager, *m_engine->m_btDynamicsWorld);
	m_thirdPersonCam.SetFollowTarget(*this);
}

void Player::Tick(float _deltaTime)
{
	AnimatedGameObject::Tick(_deltaTime);

	if (m_health <= 0)
	{
		m_body->setLinearVelocity(btVector3(0,0,0));
		return;
	};

	//Get front direction
	glm::vec3 forwardDir = m_thirdPersonCam.GetForward();
	//No y since we do not want to go up and down
	forwardDir.y = 0;
	forwardDir = glm::normalize(forwardDir);

	btVector3 currentVelocity = m_body->getLinearVelocity();

	currentVelocity.setX(0);
	currentVelocity.setZ(0);


	// Apply desired velocity, e.g., moving in the x-direction:
	glm::vec3 movement = glm::vec3(0);
	movement += forwardDir * m_movementDir.y * m_MOVEMENTSPEED;

	glm::vec3 rightDir = glm::cross(forwardDir, glm::vec3(0, 1, 0));


	movement += rightDir * m_movementDir.x * m_MOVEMENTSPEED;
	if (!m_aiming && m_sprinting) movement *= m_SPRINTMULTIPLIER;

	CheckStepping(movement * _deltaTime * m_STEPCHECKDISTANCE);
	if (glm::length(m_targetStepOffset) > 0.1f)
	{
		currentVelocity.setY(m_targetStepOffset.y * m_STEPSPEED);
		m_targetStepOffset -= m_targetStepOffset * _deltaTime * 5.f;
		m_body->setFriction(0.f);
	}
	else
	{
		m_body->setFriction(1.f);
	}

	currentVelocity.setX(movement.x);
	currentVelocity.setZ(movement.z);

	m_body->setLinearVelocity(currentVelocity);

	DoAnimation(movement);

	m_thirdPersonCam.Tick(_deltaTime);

	//Rotate model to movement
	if (m_aiming)
	{
		glm::vec3 dir = m_thirdPersonCam.GetForward();
		dir.y = 0;
		m_targetRotation = glm::quatLookAt(normalize(-dir), glm::vec3(0, 1, 0));
	}
	else
	{
		if (glm::length(movement) > 0.f)
			m_targetRotation = glm::quatLookAt(normalize(-movement), glm::vec3(0, 1, 0));
	}

	SetRotation(glm::slerp(GetRotation(), m_targetRotation, 1 - glm::exp2(-_deltaTime / m_ROTATIONLERPTIME)));
}

void Player::DoAnimation(glm::vec3 _movementInput)
{
	if (glm::length(_movementInput) <= 0.f)
	{
		if (m_aiming)
		{
			if (m_animator->GetCurrentAnimation() == m_animations[real::E_PLAYERANIMATIONS::AimWalk])
				m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::Aim], 0.25f, 9999); // Play from end
			else
				m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::Aim]);
			m_animator->SetPlayBackSpeed(4);
		}
		else
		{
			m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::Idle],0.5f);
			m_animator->SetPlayBackSpeed(1);
		}
	}
	else
	{
		if (m_aiming)
		{
			m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::AimWalk]);
			m_animator->SetPlayBackSpeed(glm::length(_movementInput) * m_WALKANIMATIONSPEED);
		}
		else
		{
			if (m_sprinting)
			{
				m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::Run]);
				m_animator->SetPlayBackSpeed(glm::length(_movementInput) * m_RUNANIMATIONSPEED);
			}
			else
			{
				m_animator->PlayAnimation(m_animations[real::E_PLAYERANIMATIONS::Walk]);
				m_animator->SetPlayBackSpeed(glm::length(_movementInput) * m_WALKANIMATIONSPEED);
			}
		}
	}
}

//Check next position and step up if needed returns if stepped
bool Player::CheckStepping(glm::vec3 _movementDir)
{
	glm::vec3 glmfrom = GetPosition();
	glmfrom.x += _movementDir.x;
	glmfrom.z += _movementDir.z;
	if (glm::length(_movementDir) != 0) _movementDir = glm::normalize(_movementDir);
	glmfrom.x += 5 * 0.135f * _movementDir.x;
	glmfrom.z += 5 * 0.135f * _movementDir.z;
	glmfrom.y -= 2.1f;
	btTransform from;
	from.setIdentity();
	from.setOrigin(GlmVecToBtVec(glmfrom + glm::vec3(0, m_STEPHEIGHT, 0)));
	btTransform to;
	to.setIdentity();
	to.setOrigin(GlmVecToBtVec(glmfrom - glm::vec3(0, m_STEPHEIGHT, 0)));

	btCollisionWorld::ClosestRayResultCallback hit = btCollisionWorld::ClosestRayResultCallback(from.getOrigin(), to.getOrigin());
	//btCollisionWorld::ClosestConvexResultCallback hit = btCollisionWorld::ClosestConvexResultCallback(from.getOrigin(), to.getOrigin());
	hit.m_collisionFilterMask = BTGROUP_ALL & ~BTGROUP_PLAYER; // Dont collide with player
	//physicsWorld->convexSweepTest(m_shape,from,to, hit);
	m_physicsWorld->rayTest(from.getOrigin(), to.getOrigin(), hit);
	
	//m_physicsWorld->getDebugDrawer()->drawLine(from.getOrigin(), to.getOrigin(), btVector3(1, 0, 1));
	//m_physicsWorld->getDebugDrawer()->drawSphere(GlmVecToBtVec(glmfrom), 0.2f, btVector3(1, 0, 1));
	if (hit.hasHit())
	{
		if (hit.m_closestHitFraction <= 0) return false;
		if (glm::dot(BtVecToGlmVec(hit.m_hitNormalWorld), BtVecToGlmVec(btVector3(0, 1, 0))) < 0.9f)
			return false;
		//m_physicsWorld->getDebugDrawer()->drawSphere(hit.m_hitPointWorld, 0.5f, btVector3(1, 0, 1));
		if (abs(hit.m_closestHitFraction - 0.5f) < m_MINSTEPHEIGHT / (m_STEPHEIGHT * 2)) return false;
		glm::vec3 newPos = BtVecToGlmVec(from.getOrigin() + (hit.m_closestHitFraction * (to.getOrigin() - from.getOrigin())));
		m_targetStepOffset = newPos - glmfrom;
		if (m_targetStepOffset.y > 0)
			m_targetStepOffset.y += m_EXTRASTEPHEIGHT;
		return true;
	}
	return false;
}

void Player::OnInputAction(real::InputAction _action)
{
	if (m_health <= 0) return;

	//Dont move if camera is in debug mode (freecam)
	if (m_thirdPersonCam.m_debugMode)
	{
		m_movementDir = glm::vec2(0);
		return;
	}

	switch (_action.action)
	{
		case VERTICAL_MOVE:
			m_movementDir.y = _action.value;
			break;
		case HORIZONTAL_MOVE:
			m_movementDir.x = _action.value;
			break;
		case SPRINT:
			m_sprinting = _action.value;
	}
	if (glm::length(m_movementDir) > 0)
		m_movementDir = glm::normalize(m_movementDir);
}

bool Player::TakeDamage(float _damage, glm::vec3 _hitLocation, glm::vec3)
{
	if (m_health <= 0) return true;

	m_health -= _damage;

	if (_hitLocation != glm::vec3(0))
	{
		glm::vec3 relativeDirection = glm::normalize(GetPosition() - _hitLocation);
		relativeDirection = m_thirdPersonCam.GetRotation() * relativeDirection; // Relative to camera for better visual
		if (m_health <= 0) relativeDirection *= 3;
		m_thirdPersonCam.AddScreenShake(relativeDirection* _damage);
	}

	if (m_health <= 0)
	{

		m_health = 0;
		
		m_inputManager->GetMouse().ChangeMouseLockMode(real::MouseLockModes::UNLOCKED);

		//How many particles do we add?
		//Yes...
		//Sadly its currently limited by the max texture size
		real::ParticleSystem* particles = new real::ParticleSystem(0.00001f, glm::vec2(0.2f, 1.00f), -1.f, true);
		particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
			, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
			, m_engine->m_loadedScene->GetMainCamera()
			, *m_engine->m_loadedScene
		);
		particles->m_spawnVelocityMinMax = glm::vec2(3.f, 5.f);
		particles->m_constantForce = glm::vec3(0, -10.f,0);
		particles->m_startPositionSize = glm::vec3(0.15f,1.7f,0.15f);
		particles->m_position = GetPosition();
		particles->m_particleSize = 0.3f;
		particles->m_startColor = glm::vec3(1.f, 0.0f, 0.f);
		particles->m_endColor = glm::vec3(0.3f, 0.0f, 0.0f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);

		//SNAKE SNAAAKE
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/GameOver.wav");
		return true;
	}
	else
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/playerhit.wav");
		return false;
	}
}

void Player::Heal(float _health)
{
	m_health = std::min(m_health + _health, m_MAX_HEALTH);
}

void Player::Shoot()
{
	printf("Try shoot\n");

	m_thirdPersonCam.AddScreenShake(20.f);

	//Spawn particles
	real::ParticleSystem* particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.03f,0.07f), 1, true);
	particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
		, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
		, m_engine->m_loadedScene->GetMainCamera()
		, *m_engine->m_loadedScene
	);
	particles->m_spawnVelocityMinMax = glm::vec2(3.f, 15.f);
	particles->m_constantForce = glm::vec3(0);
	particles->m_startPositionSize = glm::vec3(0.0f);
	particles->m_position = GetPosition() + GetRotation()*glm::vec3(0.f,1.7f,1.2f);
	particles->m_spawnDirection = m_thirdPersonCam.GetForward();
	particles->m_spawnDirectionRandomness = 0.2f;
	particles->m_particleSize = 0.1f;
	particles->m_startColor = glm::vec3(1.f, 0.3f, 0.f);
	particles->m_endColor = glm::vec3(1.f, 0.5f, 0.3f);
	m_engine->m_loadedScene->AddParticleSystem(*particles);

	LightFlash* muzzleFlash = new LightFlash(0.1f, real::E_lightTypes::pointLight, glm::vec3(1.5f, 0.8f, 0.5f), glm::vec3(0.8f, 0.6f, 0.5f), 0.05f,0.01f,0.05f);
	muzzleFlash->Init(*m_engine);
	muzzleFlash->SetPosition(GetPosition() + GetRotation() * glm::vec3(0.f, 1.7f, 1.2f));
	m_engine->m_loadedScene->GetObjectManager().AddObject(*muzzleFlash);

	btVector3 from = GlmVecToBtVec(m_thirdPersonCam.GetPosition());
	btVector3 to = GlmVecToBtVec(m_thirdPersonCam.GetPosition() + (m_thirdPersonCam.GetForward() * 100.f));
	btCollisionWorld::ClosestRayResultCallback hit(from, to);
	hit.m_collisionFilterMask = BTGROUP_ALL & ~BTGROUP_PLAYER; // Dont collide with player
	m_physicsWorld->rayTest(from, to, hit);
	if (hit.hasHit())
	{
		glm::vec3 hitPoint = BtVecToGlmVec(from + (to - from) * hit.m_closestHitFraction);

		LightFlash* hitFlash = new LightFlash(0.15f, real::E_lightTypes::pointLight, glm::vec3(1.f, 0.8f, 0.5f), glm::vec3(0.8f, 0.6f, 0.5f), 0.2f, 0.04f, 0.15f);
		hitFlash->Init(*m_engine);
		hitFlash->SetPosition(hitPoint);
		m_engine->m_loadedScene->GetObjectManager().AddObject(*hitFlash);

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
		particles->m_spawnDirection = glm::normalize(BtVecToGlmVec(to - from));
		particles->m_spawnDirectionRandomness = 0.0f;
		particles->m_position = GetPosition() + GetRotation() * glm::vec3(-0.35f, 1.6f, 2.4f);
		particles->m_particleSize = 0.15f;
		particles->m_startColor = glm::vec3(0.5f, 0.5f, 0.5f);
		particles->m_endColor = glm::vec3(0.5f, 0.5f, 0.5f);
		m_engine->m_loadedScene->AddParticleSystem(*particles);



		GameObject* hitObject = static_cast<GameObject*>(hit.m_collisionObject->getUserPointer());
		
		//Apply force
		hitObject->m_body->applyImpulse(m_SHOOT_IMPACT_FORCE * -hit.m_hitNormalWorld, GlmVecToBtVec(hitPoint - hitObject->GetPosition()));


		if (IDamageable* damagable = dynamic_cast<IDamageable*>(hitObject))
		{
			glm::vec3 dir = glm::normalize(BtVecToGlmVec(to - from));
			damagable->TakeDamage(10000,hitPoint,dir);

			//Spawn particles
			particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.2f, 0.8f), 1, true);
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
			particles->m_startColor = glm::vec3(1.0f, 0.0f, 0.0f);
			particles->m_endColor = glm::vec3(0.5f, 0.0f, 0.0f);
			m_engine->m_loadedScene->AddParticleSystem(*particles);

		}
		else
		{
			//Spawn particles
			particles = new real::ParticleSystem(50, 1.f, glm::vec2(0.2f,0.8f), 1, true);
			particles->Init(*static_cast<real::ComputeShader*>(m_engine->m_shaderManager.GetShader("particleCompute", real::E_SHADER_TYPE::COMPUTE_SHADER))
				, m_engine->m_modelManager.GetModel("../Common/Assets/Models/Particle.obj", *m_engine->m_shaderManager.GetShader("particleUnlit"))
				, m_engine->m_loadedScene->GetMainCamera()
				, *m_engine->m_loadedScene
			);
			particles->m_spawnVelocityMinMax = glm::vec2(1.f, 15.f);
			particles->m_constantForce = glm::vec3(0.f,-10.f,0.f);
			particles->m_startPositionSize = glm::vec3(0.05f);
			particles->m_spawnDirection = BtVecToGlmVec(hit.m_hitNormalWorld);
			particles->m_spawnDirectionRandomness = 0.4f;
			particles->m_position = hitPoint;
			particles->m_particleSize = 0.05f;
			particles->m_startColor = glm::vec3(1.0f, 0.9f, 0.4f);
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
		printf("Hit: x %f y %f z %f \n", hit.m_hitPointWorld.getX(), hit.m_hitPointWorld.getY(), hit.m_hitPointWorld.getZ());
	}
}

void Player::Aim()
{
	Aim(!m_aiming);
}

void Player::Aim(bool _aiming)
{
	if (_aiming)
	{
		m_thirdPersonCam.SetFollowOffset(glm::vec3(1.7, 0, 0.7f));
		m_thirdPersonCam.SetFov(50);
		m_aiming = true;
	}
	else
	{
		m_thirdPersonCam.SetFollowOffset(glm::vec3(0, 0, 0));
		m_thirdPersonCam.SetFov(90);
		m_aiming = false;
	}
}
