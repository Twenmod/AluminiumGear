#pragma once
#include "IDamageable.h"

namespace real
{
	class ObjectManager;
}

enum E_ENEMYANIMATIONS
{
	Idle,
	Walking,
	Aim,
	AimWalk,
	Die
};

class Enemy : public real::AnimatedGameObject, public IDamageable
{
public:
	Enemy();
	~Enemy();
	void Init(real::NavMesh& navMesh, real::RealEngine& engine, real::Model& model, const char* startAnimationPath, real::Model& gibModel, float mass = 0, glm::vec3 startPos = glm::vec3(0), glm::vec3 startScale = glm::vec3(1), glm::quat startRotation = glm::identity<glm::quat>());
	bool TakeDamage(float damage,glm::vec3 hitLocation = glm::vec3(0), glm::vec3 hitDirection = glm::vec3(0)) override;
	void Tick(float deltaTime);
	void SetTarget(real::GameObject& _target) { m_target = &_target; }
	void SetPatrolPath(std::vector<glm::vec3> _positions)
	{
		m_patrolPath = _positions;
		if (m_triggerTime <= 0) m_currentPath = m_navMesh->PlotPath(GetPosition(), m_patrolPath[m_currentPatrolPosition]);
	}
	bool GetIsTriggered() { return m_triggerTime > 0; }
protected:
	bool TargetLineOfSight();
	void TriggerBehaviour(float deltaTime);
	void CalmBehaviour(float deltaTime);
	void Shoot();
	real::ObjectManager* m_objectManager;
	btDynamicsWorld* m_physicsWorld;
	real::NavMesh* m_navMesh;
	real::GameObject* m_target;
	real::Model* m_gibModel;
	std::vector<real::navNode*> m_currentPath;
	std::vector<glm::vec3> m_patrolPath;
	real::TextureManager* m_textureManager;
	real::RealEngine* m_engine{ nullptr };
	unsigned int m_currentPatrolPosition{ 0 };
	float m_health{ 1 };
	float m_triggerTime{ 0 };
	bool m_seesPlayer;
	float m_shootTimer{1};
	const float m_TRIGGERINGDURATION{ 90 };
	const float m_WALKANIMSPEED{ 0.22f };
	const float m_PATROLWALKSPEED{ 4.f };
	const float m_FOLLOWINGWALKSPEED{ 8.f };
	const float m_LOOKRANGE{ 15 };
	const float m_COMBATLOOKRANGE{ 30 };
	const float m_COMBATSTOPRANGE{ 7.f };
	const float m_ROTATIONLERPTIME{ 0.1f }; // half time
	const float m_POSITIONREACHEDDISTANCE{ 2.5f };
	const float m_PATROLVIEWANGLE{ 30 };
	const float m_COMBATVIEWANGLE{ 60 };
	const float m_SHOOT_RAY_RANGE{ 100.f };
	const float m_BULLET_DAMAGE{ 12.f };
	const float m_SHOOT_INTERVAL{ 0.4f };
	const float m_INACCURACY{ 0.25f };
};