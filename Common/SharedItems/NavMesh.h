#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace real
{
	class BtDebugDrawer;

	struct navNode
	{
		glm::vec3 position;
		std::vector<navNode*> connectedNodes;
		//A* runtime values
		bool isStart;
		navNode* parent;
		float gCost;
		float hCost;
		float fCost;
		bool closed;
	};

	//A* implementation derived from https://dev.to/jansonsa/a-star-a-path-finding-c-4a4h

	inline bool operator < (const navNode& lhs, const navNode& rhs)
	{
		return lhs.fCost < rhs.fCost;
	}

	class NavMesh
	{
	public:
		NavMesh() {};
		~NavMesh();
		void Init(BtDebugDrawer* debugDrawer = nullptr);
		void load(std::string path);
		void Draw();
		void DrawPath(std::vector<navNode*> path);
		std::vector<navNode*> PlotPath(glm::vec3 startPosition, glm::vec3 endPosition);
		bool debugDraw = false;
	private:
		void ProcessAINode(aiNode* node, const aiScene* scene);
		double CalculateH(glm::vec3 _pos, navNode* _destinationNode)
		{
			glm::vec3 diff = _pos - _destinationNode->position;
			double H = glm::length(diff);
			if (diff.y > 0) H += diff.y * -m_UPWARDSCOST;
			else H += diff.y * m_DOWNWARDSCOST;
			return H;
		}
		std::vector<navNode*> MakePath(navNode* destination);
		const float m_UPWARDSCOST{ 4.f };
		const float m_DOWNWARDSCOST{-0.6f};
		//Traverse the path backwards to make a path
		std::vector<navNode> m_nodes;
		std::string m_directory;

		BtDebugDrawer* m_debugDrawer;
	};
}