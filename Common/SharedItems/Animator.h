#pragma once
//Modified from https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "BTDebugDrawer.h"
#include "Animation.h"

namespace real
{
	class Animation;

	class Animator
	{
	public:
		Animator(Animation* startAnimation);

		void Init(BtDebugDrawer* debugDrawer = nullptr);

		void UpdateAnimation(float deltaTime);
		void PlayAnimation(Animation* animation, float blendTime = 0.25f, float timeOffset = 0)
		{
			if (m_CurrentAnimation == animation) return;
			m_OldAnimation = m_CurrentAnimation;
			m_CurrentAnimation = animation;
			m_BlendTime = blendTime;
			m_BlendDuration = blendTime;
			m_OldTime = m_CurrentTime;
			m_CurrentTime = timeOffset;
		}
		void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
		std::vector<glm::mat4> GetFinalBoneMatrices() { return m_FinalBoneMatrices; }
		void SetPlayBackSpeed(float newSpeed) { m_PlayBackSpeed = newSpeed; }
		const Animation* GetCurrentAnimation() const { return m_CurrentAnimation; }
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		Animation* m_CurrentAnimation{ nullptr };
		Animation* m_OldAnimation{ nullptr };
		BtDebugDrawer* m_DebugDrawer{ nullptr };
		float m_CurrentTime{ 0 };
		float m_OldTime{ 0 };
		float m_DeltaTime{ 0 };
		float m_PlayBackSpeed{ 1 };
		float m_BlendTime{ 0 };
		float m_BlendDuration{ 0 };
	};
}