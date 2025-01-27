#include "precomp.h"
#include "TextureManager.h"
#include "Animation.h"

real::Animation::Animation(const std::string& animationPath, Model* model, bool _looping) :
	looping(_looping)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
	if (!(scene && scene->mRootNode))
	{
		printf("\x1B[31mERROR::ANIMATION::No animation root found, animation failed to load \n\x1B[37m");
		return;
	}
	auto animation = scene->mAnimations[0];
	m_Duration = static_cast<float>(animation->mDuration);
	m_TicksPerSecond = static_cast<int>(animation->mTicksPerSecond);
	ReadHeirarchyData(m_RootNode, scene->mRootNode);
	ReadMissingBones(animation, *model);
}

real::Bone* real::Animation::FindBone(const std::string& name)
{
	auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
		[&](const Bone& Bone)
		{
			return std::strcmp(Bone.GetBoneName().c_str(), name.c_str()) == 0;
		}
	);
	if (iter == m_Bones.end()) return nullptr;
	else return &(*iter);
}


void real::Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
{
	int size = animation->mNumChannels;

	auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
	int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

	//reading channels(bones engaged in an animation and their keyframes)
	for (int i = 0; i < size; i++)
	{
		auto channel = animation->mChannels[i];
		std::string boneName = channel->mNodeName.data;

		if (boneInfoMap.find(boneName) == boneInfoMap.end())
		{
			boneInfoMap[boneName].id = boneCount;
			boneCount++;
		}
		m_Bones.push_back(Bone(channel->mNodeName.data,
			boneInfoMap[channel->mNodeName.data].id, channel));
	}

	m_BoneInfoMap = boneInfoMap;
}

void real::Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
{
	assert(src);

	dest.name = src->mName.data;
	aiMatrix4x4 aitransformMatrix = src->mTransformation;
	glm::mat4 transformMatrix;
	transformMatrix[0][0] = aitransformMatrix.a1; transformMatrix[1][0] = aitransformMatrix.a2; transformMatrix[2][0] = aitransformMatrix.a3; transformMatrix[3][0] = aitransformMatrix.a4;
	transformMatrix[0][1] = aitransformMatrix.b1; transformMatrix[1][1] = aitransformMatrix.b2; transformMatrix[2][1] = aitransformMatrix.b3; transformMatrix[3][1] = aitransformMatrix.b4;
	transformMatrix[0][2] = aitransformMatrix.c1; transformMatrix[1][2] = aitransformMatrix.c2; transformMatrix[2][2] = aitransformMatrix.c3; transformMatrix[3][2] = aitransformMatrix.c4;
	transformMatrix[0][3] = aitransformMatrix.d1; transformMatrix[1][3] = aitransformMatrix.d2; transformMatrix[2][3] = aitransformMatrix.d3; transformMatrix[3][3] = aitransformMatrix.d4;
	dest.transformation = transformMatrix;

	dest.childrenCount = src->mNumChildren;

	for (unsigned int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		ReadHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}