#pragma once

class IDamageable
{
public :
	virtual bool TakeDamage(float damage, glm::vec3 hitLocation = glm::vec3(0), glm::vec3 hitDirection = glm::vec3(0)) = 0;
};