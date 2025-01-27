#pragma once


class Grenade : public real::GameObject
{
public:
	Grenade(float radius, float strength, float falloff, float timer);
	void Tick(float deltaTime) override;
	void Init(real::RealEngine& engine, real::Model& objectModel, real::E_GENERATECOLLISIONSHAPEMODE generateCollisionShape, float mass = 0, glm::vec3 startPos = glm::vec3(0), glm::vec3 startScale = glm::vec3(1), glm::quat startRotation = glm::identity<glm::quat>());
private:
	void Explode();
	const float m_RADIUS{ 10.f };
	const float m_FORCE{ 1000.f };
	const float m_FALOFF{ 0.1f };
	float m_timer{0};
	real::RealEngine* m_engine{ nullptr };
};