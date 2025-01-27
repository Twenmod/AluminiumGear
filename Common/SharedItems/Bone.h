#pragma once

//Modified from https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace real
{

	struct KeyPosition
	{
		glm::vec3 position;
		float timeStamp;
	};

	struct KeyRotation
	{
		glm::quat orientation;
		float timeStamp;
	};

	struct KeyScale
	{
		glm::vec3 scale;
		float timeStamp;
	};

	class Bone
	{
	public:

		/*reads keyframes from aiNodeAnim*/
		Bone(const std::string& name, int ID, const aiNodeAnim* channel);

		/*interpolates  b/w positions,rotations & scaling keys based on the curren time of
		the animation and prepares the local transformation matrix by combining all keys
		tranformations*/
		void Update(float animationTime);

		glm::mat4 GetLocalTransform() { return m_LocalTransform; }
		const std::string& GetBoneName() const { return m_Name; }
		int GetBoneID() const { return m_ID; }

		int GetPositionIndex(float animationTime);

		int GetRotationIndex(float animationTime);

		int GetScaleIndex(float animationTime);

	private:

		// Gets normalized value for Lerp & Slerp
		float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
		{
			float scaleFactor = 0.0f;
			float midWayLength = animationTime - lastTimeStamp;
			float framesDiff = nextTimeStamp - lastTimeStamp;
			scaleFactor = midWayLength / framesDiff;
			return scaleFactor;
		}

		// Interpolates between current keys and returns matrix
		glm::mat4 InterpolatePosition(float animationTime);

		// Interpolates between current keys and returns matrix
		glm::mat4 InterpolateRotation(float animationTime);

		// Interpolates between current keys and returns matrix
		glm::mat4 InterpolateScaling(float animationTime);
	private:
		std::vector<KeyPosition> m_Positions;
		std::vector<KeyRotation> m_Rotations;
		std::vector<KeyScale> m_Scales;
		int m_NumPositions;
		int m_NumRotations;
		int m_NumScalings;

		glm::mat4 m_LocalTransform;
		std::string m_Name;
		int m_ID;

	};

}