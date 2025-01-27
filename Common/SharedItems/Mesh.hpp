#pragma once
#include "TextureManager.h"

namespace real
{
	struct Vertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;

		glm::vec3 Tangent;
		glm::vec3 Bitangent;

		//Bone data
		int m_BoneIDs[MAX_BONE_INFLUENCE];
		float m_Weights[MAX_BONE_INFLUENCE];

	};

	struct Material
	{
		std::vector<Texture> textures;
	};


	class Shader;
	class ReflectionProbe;

	class Mesh
	{
	public:
		// mesh data
		std::vector<Vertex>       vertices;
		std::vector<unsigned int> indices;
		Material material;

		Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const Material& material, glm::vec3 center = glm::vec3(0));
		void Draw(Shader& shader, std::vector<ReflectionProbe*> activeLightProbes, bool wireframe = false);
		void DrawInstanced(Shader& shader, unsigned int instances, std::vector<ReflectionProbe*> activeLightProbes, bool wireframe = false);

		bool opaque = true;

		glm::vec3 m_centerPosition;
	private:
		//  render data
		unsigned int VAO, VBO, EBO;
		void setupMesh();
	};
}