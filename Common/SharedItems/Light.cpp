#include "precomp.h"
#include "Light.h"

real::Light::Light(E_lightTypes _type, glm::vec3 _color, glm::vec3 _specular, float _constant, float _linear, float _quadratic, float _minAngle, float _maxAngle)
{
	m_lightType = _type;
	m_constant = _constant;
	m_linear = _linear;
	m_quadratic = _quadratic;
	m_color = _color;
	m_specularColor = _specular;
	m_minAngle = _minAngle;
	m_maxAngle = _maxAngle;
}

glm::vec3 real::Light::GetDirection() const
{
	glm::vec3 direction = GetRotation() * glm::vec3(1, 0, 0);
	return direction;
}

void real::Light::Tick(float)
{

}

