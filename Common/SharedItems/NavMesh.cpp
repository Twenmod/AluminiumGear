#include "precomp.h"
#include "NavMesh.h"

#include "RealMath.h"
#include "BTDebugDrawer.h"

real::NavMesh::~NavMesh()
{
}

void real::NavMesh::Init(BtDebugDrawer* _debugDrawer)
{
	m_debugDrawer = _debugDrawer;
}

void real::NavMesh::load(std::string path)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	m_directory = path.substr(0, path.find_last_of('/'));

	ProcessAINode(scene->mRootNode, scene);
}

void real::NavMesh::ProcessAINode(aiNode* rootNode, const aiScene* scene)
{
	// Slightly hardcoded but the navmesh should be 1 mesh

	if (rootNode->mNumChildren > 1)
	{
		printf("NAV::LOAD::ERROR:TO_MANY_NODES"); abort();
	}
	aiNode* node = rootNode->mChildren[0];

	if (node->mNumMeshes > 1 || node->mNumChildren > 1)
	{
		printf("NAV::LOAD::ERROR:TO_MANY_MESHES_OR_NODES");
		abort();
	}
	aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		navNode nNode;

		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		nNode.position = vector;

		bool exists = false;
		for (navNode& existnode : m_nodes)
		{
			if (existnode.position == vector) exists = true;
		}
		if (exists) continue;

		nNode.connectedNodes = std::vector<navNode*>();

		// Push to list
		m_nodes.push_back(nNode);
	}

	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			unsigned int currentIndex = face.mIndices[j];
			navNode* currentNode = nullptr;
			glm::vec3 vertPos;
			vertPos.x = mesh->mVertices[currentIndex].x;
			vertPos.y = mesh->mVertices[currentIndex].y;
			vertPos.z = mesh->mVertices[currentIndex].z;
			for (navNode& nextnode : m_nodes)
			{
				if (nextnode.position == vertPos)
				{
					currentNode = &nextnode;
					break;
				}
			}

			// Connect to other vertices in the same face
			for (unsigned int k = 0; k < face.mNumIndices; k++)
			{
				if (j != k) // Avoid self-connection
				{
					unsigned int neighborIndex = face.mIndices[k];
					navNode* neighborNode = nullptr;

					vertPos.x = mesh->mVertices[neighborIndex].x;
					vertPos.y = mesh->mVertices[neighborIndex].y;
					vertPos.z = mesh->mVertices[neighborIndex].z;
					for (navNode& neighbour : m_nodes)
					{
						if (neighbour.position == vertPos)
						{
							neighborNode = &neighbour;
							break;
						}
					}

					// Ensure the connection doesn't already exist
					if (std::find(currentNode->connectedNodes.begin(), currentNode->connectedNodes.end(), neighborNode) == currentNode->connectedNodes.end())
					{
						currentNode->connectedNodes.push_back(neighborNode);
					}

					// Optionally, ensure bidirectional connection
					if (std::find(neighborNode->connectedNodes.begin(), neighborNode->connectedNodes.end(), currentNode) == neighborNode->connectedNodes.end())
					{
						neighborNode->connectedNodes.push_back(currentNode);
					}
				}
			}
		}
	}
}

void real::NavMesh::Draw()
{
	if (m_debugDrawer == nullptr || !debugDraw) return;
	for (navNode& node : m_nodes)
	{
		for (navNode* nChild : node.connectedNodes)
			m_debugDrawer->drawLine(GlmVecToBtVec(node.position), GlmVecToBtVec(nChild->position + glm::vec3(0, 0.1f, 0)), btVector3(0.2f, 0.2f, 1.f));
	}
}

void real::NavMesh::DrawPath(std::vector<real::navNode*> path)
{
	if (m_debugDrawer == nullptr || path.size() == 0 || !debugDraw) return;
	for (int i = 0; i < path.size() - 1; i++)
	{
		m_debugDrawer->drawLine(GlmVecToBtVec(path[i]->position + glm::vec3(0, 0.5f, 0)), GlmVecToBtVec(path[i + 1]->position + glm::vec3(0, 0.5f, 0)), btVector3(1.f, 0.3f, 0.3f));
	}
}

std::vector<real::navNode*> real::NavMesh::PlotPath(glm::vec3 startPosition, glm::vec3 endPosition)
{
	//Find start and end node
	float startDistance = std::numeric_limits<float>::max();
	int startIndex = -1;
	float destDistance = std::numeric_limits<float>::max();
	int destIndex = -1;
	for (int i = 0; i < m_nodes.size(); i++)
	{
		float sDist = glm::length(m_nodes[i].position - startPosition);
		if (sDist < startDistance)
		{
			startDistance = sDist;
			startIndex = i;
		}
		float dDist = glm::length(m_nodes[i].position - endPosition);
		if (dDist < destDistance)
		{
			destDistance = dDist;
			destIndex = i;
		}
	}
	navNode* start = &m_nodes[startIndex];
	navNode* dest = &m_nodes[destIndex];

	if (debugDraw) m_debugDrawer->drawSphere(GlmVecToBtVec(start->position), 0.24f, btVector3(0, 1, 0));

	if (debugDraw) m_debugDrawer->drawSphere(GlmVecToBtVec(dest->position), 0.24f, btVector3(0, 1, 0));


	std::vector<navNode*> empty;

	if (start == dest)
	{
		return empty;
	}

	navNode* startNode = nullptr;
	//Initialize whole map
	for (navNode& node : m_nodes)
	{
		node.fCost = FLT_MAX;
		node.gCost = FLT_MAX;
		node.hCost = FLT_MAX;
		node.parent = nullptr;
		node.isStart = false;
		node.closed = false;
		if (node.position == start->position) startNode = &node;
	}

	startNode->fCost = 0.0;
	startNode->gCost = 0.0;
	startNode->hCost = 0.0;
	startNode->isStart = true;

	std::vector<navNode*> openList;
	openList.emplace_back(startNode);

	while (!openList.empty() && openList.size() < m_nodes.size())
	{
		navNode* node = nullptr;
		float lowestFCost = FLT_MAX;
		for (navNode* n : openList)
		{
			if (n->fCost < lowestFCost)
			{
				lowestFCost = n->fCost;
				node = n;
			}
		}
		node->closed = true;
		if (debugDraw) m_debugDrawer->drawSphere(GlmVecToBtVec(node->position), 0.22f, btVector3(1, 1, 0));
		openList.erase(std::remove(openList.begin(), openList.end(), node), openList.end());

		for (navNode* connected : node->connectedNodes)
		{
			double gNew, hNew, fNew;

			if (connected->position == dest->position)
			{
				//Destination found - make path
				connected->parent = node;
				return MakePath(connected);
			}
			else if (connected->closed == false)
			{
				gNew = node->gCost + 1.0;
				hNew = CalculateH(connected->position, dest);
				fNew = gNew + hNew;
				// Check if this path is better than the one already present
				if (connected->fCost == FLT_MAX ||
					connected->fCost > fNew)
				{
					if (debugDraw) m_debugDrawer->drawSphere(GlmVecToBtVec(connected->position), 0.2f, btVector3(1, 0, 0));
					// Update the details of this neighbour node
					connected->fCost = static_cast<float>(fNew);
					connected->gCost = static_cast<float>(gNew);
					connected->hCost = static_cast<float>(hNew);
					connected->parent = node;
					openList.emplace_back(connected);
				}
			}
		}
	}
	return empty;
}

std::vector<real::navNode*> real::NavMesh::MakePath(navNode* destination)
{
	std::vector<navNode*> path;
	std::vector<navNode*> usablePath;

	navNode* node = destination;

	while (!node->isStart)
	{
		path.push_back(node);
		node = node->parent;

	}
	path.push_back(node);

	while (!path.empty())
	{
		usablePath.emplace_back(path[path.size() - 1]);
		path.pop_back();
	}
	return usablePath;
}

