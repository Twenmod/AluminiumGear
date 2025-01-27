#pragma once


class FlickeringLight : public real::Light
{
public:
	FlickeringLight(float flickerBaseStrength = 0.5f, glm::vec2 flickerInterval = glm::vec2(0.4f,10.f), glm::vec2 flickerDuration = glm::vec2(0.05f,0.4f), real::E_lightTypes type = real::E_lightTypes::pointLight, glm::vec3 color = glm::vec3(1, 1, 1), glm::vec3 specular = glm::vec3(0.2, 0.2, 0.2), float constant = 0.6, float linear = 0.03, float quadratic = 0.1, float minAngle = 30, float maxAngle = 45);
	void Init(real::RealEngine& engine);
	void Tick(float _deltaTime) override;
	glm::vec3 GetColor() const override { return m_color*m_strength; }
private:
	real::RealEngine* m_engine;
	float m_pulseTimer{ 0.f };
	float m_pulsing{ 0.f };
	float m_strength{ 0.f };
	float m_currentFlickedDuration{ 0.f };
	const float m_FLICKERING_BASE_STRENGTH{ 0.8f };
	const glm::vec2 m_FLICKERING_INTERVAL{ glm::vec2(0.4f,10.f) };
	const glm::vec2 m_FLICKER_DURATION{ glm::vec2(0.05f,0.4f) };
	const float m_FLICKER_SMOOTH_HALFTIME{ 0.1f };
};