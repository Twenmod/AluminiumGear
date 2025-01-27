#include "precomp.h"
#include "Camera.h"

real::Camera::Camera(float _screenShakeMultiplier) :
	m_SCREENSHAKE_MULTIPLIER(_screenShakeMultiplier)
{
	UpdateCameraVectors();
}

real::Camera::~Camera()
{
	if (m_inputManager != nullptr)
	{
		m_inputManager->UnSubscribe(this);
	}
}

void real::Camera::Init(InputManager* _inputManager)
{
	real::GameObject::Init();
	if (_inputManager != nullptr)
	{
		m_inputManager = _inputManager;
		m_inputManager->Subscribe(this);
	}
}

void real::Camera::Tick(float _deltaTime)
{

	//ScreenShake
	glm::vec3 force = -m_screenShakeOffset * m_SCREENSHAKE_SPRING_FORCE - m_SCREENSHAKE_SPRING_DAMPING*m_screenShakeVelocity;
	m_screenShakeVelocity += force*_deltaTime;
	m_screenShakeOffset += m_screenShakeVelocity * _deltaTime;

	if (m_inputManager != nullptr && m_debugMode)
	{
		glm::vec2 mouseOffset = m_inputManager->GetMouseDelta();
		if (glm::length(mouseOffset) > 0)
		{
			m_pitch += mouseOffset.y;
			m_yaw += mouseOffset.x;
			const float maxPitch = 80;
			m_pitch = std::max(m_pitch, -maxPitch);
			m_pitch = std::min(m_pitch, maxPitch);

			//Convert to quaternion
			glm::quat qYaw = glm::angleAxis(glm::radians(m_yaw), glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat qPitch = glm::angleAxis(glm::radians(m_pitch), glm::vec3(0.0f, 0.0f, 1.0f));
			m_rotation = qYaw * qPitch;

			UpdateCameraVectors();
		}

		if (glm::length(m_movementDir) > 0)
		{
			glm::vec2 horizontalMovement = glm::normalize(m_movementDir) * _deltaTime * m_DEBUGMOVEMENTSPEED;
			SetPosition(GetPosition() + m_front * horizontalMovement.y);
			SetPosition(GetPosition() + m_right * horizontalMovement.x);
		}
	}
}

void real::Camera::OnInputAction(InputAction _action)
{
	if (m_debugMode)
	{
		switch (_action.action)
		{
			case VERTICAL_MOVE:
				m_movementDir.y = _action.value;
				break;
			case HORIZONTAL_MOVE:
				m_movementDir.x = _action.value;
				break;
		}
	}
	else
	{
		m_movementDir = glm::vec2(0);
	}
}

glm::mat4 real::Camera::GetViewProjection() const
{
	return m_projection*GetView();
}
glm::mat4 real::Camera::GetView() const
{
	return glm::lookAt(GetPosition()+m_screenShakeOffset * m_SCREENSHAKE_MULTIPLIER, GetPosition() - m_screenShakeOffset*m_SCREENSHAKE_MULTIPLIER + m_front, m_up);
}
glm::mat4 real::Camera::GetProjection() const
{
	return m_projection;
}

void real::Camera::AddScreenShake(float impulse)
{
	float angle = (rand() / static_cast<float>(RAND_MAX)) * 360.f;
	printf("angle: %f", angle);
	glm::vec2 dir = glm::vec2(sin(glm::radians(angle)), cos(glm::radians(angle)));
	AddScreenShake(dir * impulse);
}

void real::Camera::AddScreenShake(glm::vec2 impulse)
{
	AddScreenShake(glm::vec3(impulse, 0.f));
}

void real::Camera::AddScreenShake(glm::vec3 impulse)
{
	m_screenShakeVelocity += impulse;
}
