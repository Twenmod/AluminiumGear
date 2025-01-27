#include "RealEngine.h"
#include "LightFlash.h"

LightFlash::LightFlash(float _duration, real::E_lightTypes _type, glm::vec3 _color, glm::vec3 _specular, float _constant, float _linear, float _quadratic, float _minAngle, float _maxAngle) : 
	Light(_type,_color, _specular, _constant, _linear, _quadratic, _minAngle, _maxAngle),
	m_DURATION(_duration)
{
	m_timer = m_DURATION;
}


void LightFlash::Init(real::RealEngine& _engine)
{
	m_engine = &_engine;
}

void LightFlash::Tick(float _deltaTime)
{
	m_timer -= _deltaTime;
	if (m_timer <= 0)
	{
		//Die
		m_engine->m_loadedScene->GetObjectManager().RemoveObject(*this);
		return;
	}
}
