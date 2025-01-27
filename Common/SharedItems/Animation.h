#pragma once
//Modified from https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Bone.h"
#include "TextureManager.h"
#include "Model.hpp"

namespace real
{
	class Model;

	struct AssimpNodeData
	{
		glm::mat4 transformation;
		std::string name;
		int childrenCount;
		std::vector<AssimpNodeData> children;
	};

	class Animation
	{
	public:
		Animation() = default;

		Animation(const std::string& animationPath, Model* model, bool looping = true);

		~Animation()
		{
		}

		Bone* FindBone(const std::string& name);

		inline float GetTicksPerSecond() { return static_cast<float>(m_TicksPerSecond); }

		inline float GetDuration() { return m_Duration; }

		inline const AssimpNodeData& GetRootNode() { return m_RootNode; }

		inline std::map<std::string, BoneInfo>& GetBoneIDMap()
		{
			return m_BoneInfoMap;
		}
		const bool GetLooping() const { return looping; }
	private:
		void ReadMissingBones(const aiAnimation* animation, Model& model);

		void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

		float m_Duration;
		int m_TicksPerSecond;
		const bool looping;
		std::vector<Bone> m_Bones;
		AssimpNodeData m_RootNode;
		std::map<std::string, BoneInfo> m_BoneInfoMap;
	};
}