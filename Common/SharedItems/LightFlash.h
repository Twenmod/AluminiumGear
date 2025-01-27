#pragma once


class LightFlash : public real::Light
{
public:
	LightFlash(float duration = 0.3f, real::E_lightTypes type = real::E_lightTypes::pointLight, glm::vec3 color = glm::vec3(1, 1, 1), glm::vec3 specular = glm::vec3(0.2, 0.2, 0.2), float constant = 0.6, float linear = 0.03, float quadratic = 0.1, float minAngle = 30, float maxAngle = 45);
	void Init(real::RealEngine& engine);
	void Tick(float _deltaTime) override;
private:
	real::RealEngine* m_engine{nullptr};
	const float m_DURATION{ 0.5f };
	float m_timer{ 0 };

};