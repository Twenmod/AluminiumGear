#pragma once
#include "precomp.h"

glm::vec3 BtVecToGlmVec(const btVector3& vector);
btVector3 GlmVecToBtVec(const glm::vec3& vector);

glm::quat BtQuatToGlmQuat(const btQuaternion& quaternion);
btQuaternion GlmQuatToBtQuat(const glm::quat& quaternion);