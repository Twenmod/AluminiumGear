#pragma once
#include "AnimatedGameObject.h"
#include "ThirdPersonCamera.hpp"
#include "IDamageable.h"

namespace real
{
	class Animator;

	enum E_PLAYERANIMATIONS
	{
		Idle,
		Walk,
		Run,
		Aim,
		AimWalk,
		Punch,
		Die
	};

}

class Player : public real::AnimatedGameObject, real::IInputEvent, public IDamageable
{
public:
	Player();
	~Player();
	void Init(real::InputManager& inputManager, real::Model& model, real::RealEngine& engine);
	void Tick(float deltaTime);
	void OnInputAction(real::InputAction action);
	bool TakeDamage(float damage, glm::vec3 hitLocation = glm::vec3(0), glm::vec3 hitDirection = glm::vec3(0)) override;
	void Heal(float health);
	void Shoot();
	void Aim();
	void Aim(bool aiming);
	float GetHealth() const { return m_health; }
	bool GetIsAiming() const { return m_aiming; }
	ThirdPersonCamera m_thirdPersonCam;
private:
	void DoAnimation(glm::vec3 movementInput);
	bool CheckStepping(glm::vec3 movementDir);
	real::InputManager* m_inputManager{ nullptr };
	btDynamicsWorld* m_physicsWorld{ nullptr };
	real::RealEngine* m_engine{ nullptr };
	//TODO: First person camera
	const float m_MOVEMENTSPEED{ 6.f };
	const float m_SPRINTMULTIPLIER{ 1.8f };
	glm::vec2 m_movementDir{ glm::vec2(0.f) };
	glm::quat m_targetRotation{ glm::identity<glm::quat>() };
	btCylinderShape* m_shape{ nullptr };
	const float m_ROTATIONLERPTIME{ 0.05f }; // Half time
	const float m_WALKANIMATIONSPEED = 0.2f;
	const float m_RUNANIMATIONSPEED{ 0.1f };
	const float m_STEPHEIGHT{ 2.0f };
	const float m_MINSTEPHEIGHT{ 0.1f };
	const float m_STEPCHECKDISTANCE{ 1.f };
	const float m_EXTRASTEPHEIGHT{ 1.f };
	const float m_STEPSPEED{ 10.f };
	const float m_SHOOT_IMPACT_FORCE{ 2000.f };
	glm::vec3 m_targetStepOffset{ glm::vec3(0.f) };
	bool m_sprinting{ false };
	bool m_aiming{ false };
	const float m_MAX_HEALTH{ 100.f };
	float m_health{ 100 };
};