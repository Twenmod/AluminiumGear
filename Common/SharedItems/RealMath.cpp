#include "RealMath.h"

glm::vec3 BtVecToGlmVec(const btVector3& _vector)
{
	return glm::vec3(_vector.getX(),_vector.getY(),_vector.getZ());
}

btVector3 GlmVecToBtVec(const glm::vec3& _vector)
{
	return btVector3(_vector.x, _vector.y, _vector.z);
}


glm::quat BtQuatToGlmQuat(const btQuaternion& _quaternion)
{
	return glm::quat(_quaternion.w(), _quaternion.x(), _quaternion.y(), _quaternion.z());
}

btQuaternion GlmQuatToBtQuat(const glm::quat& _quaternion)
{
	return btQuaternion(_quaternion.x, _quaternion.y, _quaternion.z, _quaternion.w);
}