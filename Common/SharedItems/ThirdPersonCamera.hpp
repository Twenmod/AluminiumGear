#pragma once
#include "Camera.h"

namespace real
{
	class InputManager;
}

class ThirdPersonCamera : public real::Camera
{
public:
	ThirdPersonCamera();
	void Init(real::InputManager& inputManager, btDynamicsWorld& physicsWorld);
	void Tick(float deltaTime);
	void OnInputAction(real::InputAction action);
	void SetFollowTarget(GameObject& target) { m_followTarget = &target; }
	//Lerp to new offset relative to camera rotation
	void SetFollowOffset(glm::vec3 offset) { m_cameraFollowOffset = offset; }
	void SetFov(float _fov) { m_targetFov = _fov; }
private:
	btDynamicsWorld* m_physicsWorld{ nullptr };
	GameObject* m_followTarget{ nullptr };
	glm::vec3 m_cameraFollowOffset = { glm::vec3(0.f, 0.f, 0.f) };
	glm::vec3 m_cameraOffset = { glm::vec3(0.f, 0.f, 0.f) };
	const glm::vec3 m_TARGETOFFSET{ glm::vec3(0.f, 2.f, 0.f) };
	const float m_FOLLOWOFFSETLERPTIME{ 0.25f };
	const float m_FALLBACKLERP{ 0.25f }; // Half time
	const float m_SHOULDEROFFSET{ 0.0f };
	const float m_TARGETORBITDISTANCE{ 5.5 };
	const float m_COLLISIONMARGIN{ 0.5f };
	float m_orbitDistance{ 0.1f };
	float m_targetFov{ 90 };
	float m_fov{ 0 };
	float m_fovLerpTime{ 0.1f };
	float m_pitch{ 0 };
	float m_yaw{ 0 };
};