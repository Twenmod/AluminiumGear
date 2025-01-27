#pragma once

#include "GameObject.h"

namespace real
{
	class RealEngine;

	enum class E_lightTypes
	{
		directionalLight,
		pointLight,
		spotLight,
	};

	class Light : public GameObject
	{
	public:
		Light() {};
		Light(E_lightTypes type, glm::vec3 color = glm::vec3(1, 1, 1), glm::vec3 specular = glm::vec3(0.2, 0.2, 0.2), float constant = 0.6, float linear = 0.03, float quadratic = 0.1, float minAngle = 30, float maxAngle = 45);
		~Light() {};
		glm::vec3 GetDirection() const;

		void Tick(float deltaTime) override;
		E_lightTypes GetType() const { return m_lightType; }
		float GetConstant() const { return m_constant; }
		float GetLinear() const { return m_linear; }
		float GetQuadratic() const { return m_quadratic; }

		float GetMinAngle() const { return m_minAngle; }
		float GetMaxAngle() const { return m_maxAngle; }
		virtual glm::vec3 GetColor() const { return m_color; }
		glm::vec3 GetSpecular() const { return m_specularColor; }
	protected:
		E_lightTypes m_lightType = E_lightTypes::pointLight;
		float m_constant{ 10.f };
		float m_linear{ 3.f };
		float m_quadratic{ 2.f };
		float m_minAngle{ 30.f };
		float m_maxAngle{ 45.f };
		glm::vec3 m_color{ glm::vec3(1.f, 1.f, 1.f) };
		glm::vec3 m_specularColor{ glm::vec3(1.f, 1.f, 1.f) };

	};
}