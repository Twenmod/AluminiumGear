#pragma once
#include "GameObject.h"
#include "Animator.h"

namespace real
{
	class Animation;

	class AnimatedGameObject : public GameObject
	{
	public:
		AnimatedGameObject();
		virtual ~AnimatedGameObject();
		void Init(Model& model, const char* startAnimationPath, E_GENERATECOLLISIONSHAPEMODE generateCollisionShape = E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE, float mass = 0, glm::vec3 startPos = glm::vec3(0), glm::vec3 startScale = glm::vec3(1), glm::quat startRotation = glm::identity<glm::quat>());
		void Tick(float deltaTime);
		void SetupDraw(glm::mat4 viewProjection, std::vector<ReflectionProbe*> activeReflectionProbes = std::vector<ReflectionProbe*>(), bool wireframe = false);
		real::Animation& LoadAnimation(const char* animationPath, unsigned int _animationListPosition, bool looping = true);
	protected:
		Animator* m_animator{ nullptr };
		std::vector<Animation*> m_animations;
	};
}