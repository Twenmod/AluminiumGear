#include "precomp.h"
#include "TextureManager.h"
#include "Animator.h"

#include "Animation.h"

real::Animator::Animator(Animation* _animation)
{
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = _animation;

		m_FinalBoneMatrices.reserve(MAX_BONES);

		for (int i = 0; i < MAX_BONES; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}
}

void real::Animator::Init(BtDebugDrawer* _debugDrawer)
{
	m_DebugDrawer = _debugDrawer;
}

void real::Animator::UpdateAnimation(float _deltaTime)
{
	m_DeltaTime = _deltaTime *= m_PlayBackSpeed;
	if (m_CurrentAnimation)
	{
		m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * _deltaTime;
		//Loop or clamp
		if (m_CurrentAnimation->GetLooping())
			m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
		else
			m_CurrentTime = std::min(m_CurrentTime, m_CurrentAnimation->GetDuration());

		//Blend between old animation if switching
		if (m_OldAnimation && m_BlendTime > 0)
		{
			m_OldTime += m_OldAnimation->GetTicksPerSecond() * _deltaTime;
			//Loop or clamp
			if (m_OldAnimation->GetLooping())
				m_OldTime = fmod(m_OldTime, m_OldAnimation->GetDuration());
			else
				m_OldTime = std::min(m_OldTime, m_OldAnimation->GetDuration());
			m_BlendTime -= _deltaTime;
		}

		CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
	}
}



void real::Animator::CalculateBoneTransform(const AssimpNodeData* _node, glm::mat4 _parentTransform)
{
	std::string nodeName = _node->name;
	glm::mat4 nodeTransform = _node->transformation;

	Bone* bone = m_CurrentAnimation->FindBone(nodeName);

	if (bone)
	{
		bone->Update(m_CurrentTime);
		if (m_BlendTime > 0 && m_OldAnimation != nullptr)
		{
			//Blend animations
			Bone* oldBone = m_OldAnimation->FindBone(nodeName);
			if (oldBone)
			{
				oldBone->Update(m_OldTime);
				float a = 1 - (m_BlendTime / m_BlendDuration);
				nodeTransform =(oldBone->GetLocalTransform())*(1-a) + (bone->GetLocalTransform()*a);
			}
		}
		else
		{
			nodeTransform = bone->GetLocalTransform();
		}
	}

	glm::mat4 globalTransformation = _parentTransform * nodeTransform;

	glm::mat4 debugPos = glm::mat4(1);

	auto& boneInfoMap = m_CurrentAnimation->GetBoneIDMap();

	if (boneInfoMap.find(nodeName) != boneInfoMap.end())
	{
		int index = boneInfoMap[nodeName].id;
		glm::mat4 offset = boneInfoMap[nodeName].offset;
		m_FinalBoneMatrices[index] = globalTransformation * offset;
		debugPos = globalTransformation * offset;

	}

	for (int i = 0; i < _node->childrenCount; i++)
	{
		CalculateBoneTransform(&_node->children[i], globalTransformation);
		if (m_DebugDrawer != nullptr && debugPos != glm::mat4(1))
		{
			float x = globalTransformation[3][0];
			float y = globalTransformation[3][1];
			float z = globalTransformation[3][2];
			glm::mat4 childTrans = globalTransformation * _node->children[i].transformation;
			float x2 = childTrans[3][0];
			float y2 = childTrans[3][1];
			float z2 = childTrans[3][2];
			m_DebugDrawer->drawLine(btVector3(x, y, z), btVector3(x2, y2, z2), btVector3(0, 1, 0));
		}
	}
}