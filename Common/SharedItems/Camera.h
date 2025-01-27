#pragma once
#include "IInputEvent.h"
#include "GameObject.h"

class InputManager;

namespace real
{

	//Based learnopengl camera

	class Camera : public GameObject, public IInputEvent
	{
	public:
		Camera(float screenShakeMultipler = 0.1f);
		~Camera();
		/// <summary>
		/// Initializes the camera
		/// </summary>
		/// <param name="inputManager">, Optional reference to input, By default only necesary when you want to use debugmode on this camera </param>
		virtual void Init(InputManager* inputManager = nullptr);
		virtual void Tick(float deltaTime);
		virtual void OnInputAction(InputAction action) override;
		glm::mat4 GetViewProjection() const;
		glm::mat4 GetView() const;
		glm::mat4 GetProjection() const;
		glm::vec3 GetForward() const { return m_front; }
		glm::vec3 GetRight() const { return m_right; }
		void SetRotation(glm::quat _quaternionRotation) override
		{
			m_rotation = _quaternionRotation;
			UpdateCameraVectors();
		}
		void SetCameraVectors(glm::vec3 _front, glm::vec3 _up, glm::vec3 _right)
		{
			m_front = _front;
			m_up = _up;
			m_right = _right;
		}
		void SetProjection(glm::mat4 _newProjection) { m_projection = _newProjection; }
		bool m_debugMode{ true };

		void AddScreenShake(float force);
		void AddScreenShake(glm::vec2 impulse);
		void AddScreenShake(glm::vec3 impulse);

	protected:
		void UpdateCameraVectors()
		{
			m_front = m_rotation * glm::vec3(1, 0, 0);
			m_right = glm::normalize(glm::cross(m_front, m_WORLDUPDIRECTION));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
			m_up = glm::normalize(glm::cross(m_right, m_front));
		}
		InputManager* m_inputManager{nullptr};
		glm::quat m_rotation;
		glm::vec3 m_front;
		glm::vec3 m_up;
		glm::vec3 m_right;
		glm::mat4 m_projection;
		const glm::vec3 m_WORLDUPDIRECTION{ glm::vec3(0, 1, 0) };

		const float m_SCREENSHAKE_SPRING_FORCE{ 700.f };
		const float m_SCREENSHAKE_SPRING_DAMPING{ 10.0f };
		const float m_SCREENSHAKE_MULTIPLIER{ 0.1f };

		glm::vec3 m_screenShakeOffset{ glm::vec3(0) };
		glm::vec3 m_screenShakeVelocity{ glm::vec3(0) };
		
	private:
		// calculates the front vector from the Camera's (updated) Euler Angles
		const float m_DEBUGMOVEMENTSPEED{ 20 };
		glm::vec2 m_movementDir{ glm::vec2(0) };
		float m_pitch{ 0 };
		float m_yaw{ 0 };

	};
}