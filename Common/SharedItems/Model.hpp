#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.hpp"
#include "TextureManager.h"

//Slightly modified from LearnOpengl
//https://learnopengl.com/Model-Loading/Model
//https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation

class btCompoundShape;
class btTriangleMesh;

namespace real
{
	class Shader;

	enum class Primitives
	{
		PRIMITIVE_PLANE,
		PRIMITIVE_CUBE
	};

	struct BoneInfo
	{
		int id;
		glm::mat4 offset;

	};

	class Model
	{
	public:
		Model() {}
		~Model();
		void Init(TextureManager* textureManager, const char* path, Shader& shader);
		void Init(TextureManager* textureManager, Primitives model, Texture& texture, Shader& shader);
		void Init(Mesh& mesh, Shader& _shader, TextureManager* _textureManager);
		//void LoadCubeMap();

		//Draws all this models meshes unused
		void Draw(std::vector<ReflectionProbe*> activeReflectionProbes, bool wireframe = false);

		void DrawInstanced(unsigned int instanceAmount, std::vector<ReflectionProbe*> activeReflectionProbes, bool wireframe = false);

		std::vector<Mesh*> GetMeshes() { return meshes; }

		//Pretty ineneficient so use sparingly
		std::vector<glm::vec3> GetAllVertices();
		//Pretty ineneficient so use sparingly
		std::vector<unsigned int> GetAllIndices();

		btCompoundShape* loadModelShape(bool convex = false, glm::vec3 offset = glm::vec3(0));

		std::map<std::string, real::BoneInfo>& GetBoneInfoMap() { return m_BoneInfoMap; }
		int& GetBoneCount() { return m_BoneCounter; }

		const Shader& GetShader() const { return *m_shader; }

		//void SetupInstanceData(unsigned int dataBuffer, unsigned int location, unsigned int size = 3, void* offset = (void*)0);
	private:
		// model data
		std::vector<Mesh*> meshes;
		std::string directory;

		btCompoundShape* modelShape{ nullptr };
		std::vector<btTriangleMesh*> triangleMeshes;
		std::vector<btCollisionShape*> collisionMeshes;

		TextureManager* m_textureManager{ nullptr };

		void loadModel(std::string path);
		void loadModel(Primitives model, Texture texture);
		void processNode(aiNode* node, const aiScene* scene);
		Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
		std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, E_BASE_TEXTURE_TYPES texture_type);
		std::map<std::string, BoneInfo> m_BoneInfoMap;
		int m_BoneCounter = 0;
		Shader* m_shader{ nullptr };

		void SetVertexBoneDataToDefault(Vertex& vertex)
		{
			for (int i = 0; i < MAX_BONE_WEIGHTS; i++)
			{
				vertex.m_BoneIDs[i] = -1;
				vertex.m_Weights[i] = 0.0f;
			}
		}
		void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
		{
			for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
			{
				if (vertex.m_BoneIDs[i] < 0)
				{
					vertex.m_Weights[i] = weight;
					vertex.m_BoneIDs[i] = boneID;
					break;
				}
			}
		}
		void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
	};
}