#include "precomp.h"
#include "TextureManager.h"
#include "Shader.h"

#include "Mesh.hpp"
#include "ReflectionProbe.h"

real::Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const Material& material, glm::vec3 _center)
{
	this->vertices = vertices;
	this->indices = indices;
	this->material = material;
	m_centerPosition = _center;

	setupMesh();
}

void real::Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	if (indices.size() > 0)
	{
		glGenBuffers(1, &EBO);
	}

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	if (indices.size() > 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
			&indices[0], GL_STATIC_DRAW);
	}

	// vertex positions

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	// vertex tangents
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
	// vertex bitangents
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
	// vertex bone ids
	glEnableVertexAttribArray(5);
	glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));
	// vertex weights
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));

	glBindVertexArray(0);
}

void real::Mesh::Draw(Shader& shader, std::vector<ReflectionProbe*> activeLightProbes, bool wireframe)
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int emissiveNr = 1;
	unsigned int normalNr = 1;
	unsigned int alphaNr = 1;
	for (unsigned int i = 0; i < material.textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		E_BASE_TEXTURE_TYPES type = material.textures[i].type;

		std::string typeString = "";
		if (type == TEXTURE_TYPE_DIFFUSE)
		{
			number = std::to_string(diffuseNr++);
			typeString = "diffuse";
		}
		else if (type == TEXTURE_TYPE_SPECULAR)
		{
			number = std::to_string(specularNr++);
			typeString = "specular";
		}
		else if (type == TEXTURE_TYPE_EMISSIVE)
		{
			number = std::to_string(emissiveNr++);
			typeString = "emission";
		}			
		else if (type == TEXTURE_TYPE_ALPHA)
		{
			number = std::to_string(alphaNr++);
			typeString = "alpha";
		}
		else if (type == TEXTURE_TYPE_NORMAL)
		{
			number = std::to_string(normalNr++);
			typeString = "normal";
		}

		std::string texpath = ("material." + typeString);
		//std::string texpath = "s_texture";
		shader.setInt(texpath.c_str(), i);
		glBindTexture(GL_TEXTURE_2D, material.textures[i].id);

		shader.setBool("material.hasSpec", specularNr > 1);
		shader.setBool("material.hasEmission", emissiveNr > 1);
		shader.setBool("material.hasAlpha", alphaNr > 1);
		shader.setBool("material.hasNormal", normalNr > 1);
	}
	if (activeLightProbes.size() < 2)
	{
		shader.setInt("cubemap[" + std::to_string(0) + "]", CUBEMAPTEXTUREPOSITION + 0);
		shader.setInt("cubemap[" + std::to_string(1) + "]", CUBEMAPTEXTUREPOSITION + 1);
	}
	for (unsigned int i = 0; i < activeLightProbes.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + CUBEMAPTEXTUREPOSITION + i);
		shader.setInt("cubemap["+std::to_string(i)+"]", CUBEMAPTEXTUREPOSITION + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, activeLightProbes[i]->GetCubemap().id);
	}

	if (material.textures.size() > 0)
		glActiveTexture(GL_TEXTURE0);

	// draw mesh
	glBindVertexArray(VAO);
	if (indices.size() > 0)
	{
		if (wireframe) 	glDrawElements(GL_LINE_LOOP, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
		else glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0);
	}
	else
	{
		if (wireframe) glDrawArrays(GL_LINE_LOOP, 0, (GLsizei)vertices.size());
		else glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertices.size());
	}
	glBindVertexArray(0);

}

void real::Mesh::DrawInstanced(Shader& _shader, unsigned int _instances, std::vector<ReflectionProbe*> _activeLightProbes, bool  )
{
	unsigned int diffuseNr = 1;
	unsigned int specularNr = 1;
	unsigned int emissiveNr = 1;
	unsigned int alphaNr = 1;
	for (unsigned int i = 0; i < material.textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i); // activate proper texture unit before binding
		// retrieve texture number (the N in diffuse_textureN)
		std::string number;
		E_BASE_TEXTURE_TYPES type = material.textures[i].type;

		std::string typeString = "";
		if (type == TEXTURE_TYPE_DIFFUSE)
		{
			number = std::to_string(diffuseNr++);
			typeString = "diffuse";
		}
		else if (type == TEXTURE_TYPE_SPECULAR)
		{
			number = std::to_string(specularNr++);
			typeString = "specular";
		}
		else if (type == TEXTURE_TYPE_EMISSIVE)
		{
			number = std::to_string(emissiveNr++);
			typeString = "emission";
		}
		else if (type == TEXTURE_TYPE_ALPHA)
		{
			number = std::to_string(alphaNr++);
			typeString = "alpha";
		}

		std::string texpath = ("material." + typeString);
		//std::string texpath = "s_texture";
		_shader.setInt(texpath.c_str(), i);
		glBindTexture(GL_TEXTURE_2D, material.textures[i].id);

		_shader.setBool("material.hasSpec", specularNr > 1);
		_shader.setBool("material.hasEmission", emissiveNr > 1);
		_shader.setBool("material.hasAlpha", alphaNr > 1);
	}
	if (_activeLightProbes.size() < 2)
	{
		_shader.setInt("cubemap[" + std::to_string(0) + "]", CUBEMAPTEXTUREPOSITION + 0);
		_shader.setInt("cubemap[" + std::to_string(1) + "]", CUBEMAPTEXTUREPOSITION + 1);
	}
	for (unsigned int i = 0; i < _activeLightProbes.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + CUBEMAPTEXTUREPOSITION + i);
		_shader.setInt("cubemap[" + std::to_string(i) + "]", CUBEMAPTEXTUREPOSITION + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _activeLightProbes[i]->GetCubemap().id);
	}

	if (material.textures.size() > 0)
		glActiveTexture(GL_TEXTURE0);

	// draw mesh
	glBindVertexArray(VAO);
	if (indices.size() > 0)
	{
		glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, 0, _instances);
	}
	else
	{
		glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)vertices.size(), _instances);
	}
	glBindVertexArray(0);
}