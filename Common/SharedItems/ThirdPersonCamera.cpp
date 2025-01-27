#include "RealEngine.h"
#include "ThirdPersonCamera.hpp"


ThirdPersonCamera::ThirdPersonCamera() : Camera()
{
	m_debugMode = false;
}

void ThirdPersonCamera::Init(real::InputManager& _inputManager, btDynamicsWorld& _physicsWorld)
{
	real::Camera::Init(&_inputManager);
	m_physicsWorld = &_physicsWorld;
}

void ThirdPersonCamera::Tick(float _deltaTime)
{
	real::Camera::Tick(_deltaTime);
	if (m_debugMode) return;
	
	if (abs(m_fov - m_targetFov) > 0.1f)
	{
		m_fov = glm::mix(m_fov, m_targetFov, 1 - exp2(-_deltaTime / m_fovLerpTime));
		SetProjection(glm::perspective(glm::radians(m_fov), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 120.0f));
	}
	
	//Rotation
	glm::vec2 mouseOffset = m_inputManager->GetMouseDelta();
	if (glm::length(mouseOffset) > 0)
	{
		m_pitch += mouseOffset.y;
		m_yaw += mouseOffset.x;
		m_yaw = fmod(m_yaw, 360.f);
		const float maxPitch = 80;
		m_pitch = std::max(m_pitch, -maxPitch);
		m_pitch = std::min(m_pitch, maxPitch);

		//Convert to quaternion
		glm::quat qYaw = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat qPitch = glm::angleAxis(glm::radians(m_pitch), glm::vec3(0.0f, 0.0f, 1.0f));
		m_rotation = qYaw * qPitch;
		UpdateCameraVectors();
	}

	m_cameraOffset = glm::mix(m_cameraOffset, m_cameraFollowOffset, 1 - exp2(-_deltaTime / m_FOLLOWOFFSETLERPTIME));

	//Set Position with collisionCheck
	btVector3 from = GlmVecToBtVec(m_followTarget->GetPosition()+m_TARGETOFFSET+ m_rotation*m_cameraOffset);
	btVector3 to = from - GlmVecToBtVec(m_front * m_orbitDistance);
	btCollisionWorld::ClosestRayResultCallback hit(from, to);
	hit.m_collisionFilterMask = BTGROUP_ALL &~ BTGROUP_PLAYER; // Dont collide with player
	m_physicsWorld->rayTest(from, to, hit);
	if (hit.hasHit())
	{
		float hitDistance = glm::length(BtVecToGlmVec(hit.m_hitPointWorld - from));
		if (hitDistance <= m_orbitDistance-m_COLLISIONMARGIN) m_orbitDistance = hitDistance+m_COLLISIONMARGIN;
		if (m_orbitDistance > m_TARGETORBITDISTANCE) m_orbitDistance = m_TARGETORBITDISTANCE;
	}
	else
	{
		// Lerp orbitSize
		float a = 1 - exp2(-_deltaTime / m_FALLBACKLERP);
		m_orbitDistance = m_orbitDistance * (1 - a) + m_TARGETORBITDISTANCE * a;
	}

	SetPosition(BtVecToGlmVec(from) - m_front * (m_orbitDistance-m_COLLISIONMARGIN*2));

}

void ThirdPersonCamera::OnInputAction(real::InputAction _action)
{
	real::Camera::OnInputAction(_action);
}