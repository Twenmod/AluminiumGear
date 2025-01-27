#include "precomp.h"
#include <iostream>
#include <vector>

#include "Shader.h"
#include "TextureManager.h"
#include "Model.hpp"

#include "btBulletDynamicsCommon.h"
#include "btBulletCollisionCommon.h"

#include "RealMath.h"

real::Model::~Model()
{
	for (int i = 0; i < meshes.size(); i++)
	{
		delete meshes[i];
	}
	//Delete collisionshape if loaded
	if (modelShape != nullptr)
	{
		//Delete subshapes
		for (int i = 0; i < modelShape->getNumChildShapes(); i++)
		{
			btCollisionShape* childShape = modelShape->getChildShape(i);
			delete childShape;
		}
		//and their meshes (why is this not owned by the shape >:( )
		for (int i = 0; i < triangleMeshes.size(); i++)
		{
			delete triangleMeshes[i];
		}

		delete modelShape;
	}
}

void real::Model::Init(TextureManager* _textureManager, const char* path, Shader& _shader)
{
	m_shader = &_shader;
	m_textureManager = _textureManager;
	loadModel(path);
}
void real::Model::Init(TextureManager* _textureManager, Primitives model, Texture& texture, Shader& _shader)
{
	m_shader = &_shader;
	m_textureManager = _textureManager;
	loadModel(model, texture);
}

void real::Model::Init(Mesh& _mesh, Shader& _shader, TextureManager* _textureManager)
{
	m_shader = &_shader;
	m_textureManager = _textureManager;
	meshes.push_back(&_mesh);
}

void real::Model::Draw(std::vector<ReflectionProbe*> activeReflectionProbes, bool wireframe)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i]->Draw(*m_shader,activeReflectionProbes, wireframe);
}

void real::Model::DrawInstanced(unsigned int _instanceAmount, std::vector<ReflectionProbe*> _activeReflectionProbes, bool _wireframe)
{
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i]->DrawInstanced(*m_shader, _instanceAmount, _activeReflectionProbes, _wireframe);
}

std::vector<glm::vec3> real::Model::GetAllVertices()
{
	std::vector<glm::vec3> vertices;
	for (real::Mesh* mesh : meshes)
	{
		for (Vertex& vert : mesh->vertices)
		{
			vertices.push_back(vert.Position);
		}
	}
	return vertices;
}
std::vector<unsigned int> real::Model::GetAllIndices()
{
	std::vector<unsigned int> indices;
	for (real::Mesh* mesh : meshes)
	{
		for (unsigned int& indi : mesh->indices)
		{
			indices.push_back(indi);
		}
	}
	return indices;
}

btCompoundShape* real::Model::loadModelShape(bool convex, glm::vec3 _offset)
{

	if (modelShape != nullptr)
	{
		return modelShape; // Might not load the correct one i.e convex non convex but works for now

		////Delete subshapes
		//for (int i = 0; i < modelShape->getNumChildShapes(); i++)
		//{
		//	btCollisionShape* childShape = modelShape->getChildShape(i);
		//	delete childShape;
		//}
		////and their meshes (why is this not owned by the shape >:( )
		//for (int i = 0; i < triangleMeshes.size(); i++)
		//{
		//	delete triangleMeshes[i];
		//}
		//triangleMeshes.resize(0);

		//delete modelShape;
	}

	btCompoundShape* compoundShape = new btCompoundShape();

	//Add collision for all meshes to combined shape

	for (const Mesh* mesh : meshes)
	{

		std::vector<Vertex> vertices = mesh->vertices;
		std::vector<unsigned int> indices = mesh->indices;

		if (indices.size() <= 0)
		{
			std::cout << "\x1B[31mERROR::BT::MESH::LOAD: NO INDICES FOUND\n\x1B[37m";
			continue;
		}

		btCollisionShape* meshShape;

		if (convex)
		{
			std::vector<btVector3> addedVerts;
			btConvexHullShape* hullShape = new btConvexHullShape();

			for (int i = 0; i < indices.size(); i += 3)
			{
				if (i + 3 > indices.size()) continue;
				for (int j = 0; j < 3; j++)
				{
					int idx = indices[i + j];
					const btVector3 vertex(
						vertices[idx].Position.x + _offset.x,
						vertices[idx].Position.y + _offset.y,
						vertices[idx].Position.z + _offset.z
					);
					//Check if the vertex is unique
					if (std::find(addedVerts.begin(),addedVerts.end(),vertex) == addedVerts.end())
					{
						hullShape->addPoint(vertex);
						addedVerts.push_back(vertex);
					}
				}
			}
			hullShape->optimizeConvexHull();
			hullShape->recalcLocalAabb();
			meshShape = hullShape;
			collisionMeshes.push_back(hullShape);
		}
		else
		{
			btTriangleMesh* triangleMesh = new btTriangleMesh();

			//Add per triangle
			for (int i = 0; i < indices.size(); i += 3)
			{
				if (indices.size() - i < 3) continue;
				btVector3 vertex0(vertices[indices[i]].Position.x+_offset.x, vertices[indices[i]].Position.y + _offset.y, vertices[indices[i]].Position.z + _offset.z);
				btVector3 vertex1(vertices[indices[i + 1]].Position.x+_offset.x, vertices[indices[i + 1]].Position.y + _offset.y, vertices[indices[i + 1]].Position.z + _offset.z);
				btVector3 vertex2(vertices[indices[i + 2]].Position.x + _offset.x, vertices[indices[i + 2]].Position.y + _offset.y, vertices[indices[i + 2]].Position.z + _offset.z);
				triangleMesh->addTriangle(vertex0, vertex1, vertex2);
			}
			meshShape = new btBvhTriangleMeshShape(triangleMesh, true);
			//Save for later deletion
			triangleMeshes.push_back(triangleMesh);

		}
		btTransform transform;
		transform.setIdentity();

		compoundShape->addChildShape(transform, meshShape);
	}

	modelShape = compoundShape;
	return modelShape;
}

//void Model::DrawInstanced(Shader& shader, unsigned int instances)
//{
//	for (unsigned int i = 0; i < meshes.size(); i++)
//		meshes[i].DrawInstanced(shader, instances);
//}
//
//void Model::SetupInstanceData(unsigned int dataBuffer, unsigned int location, unsigned int size, void* offset)
//{
//	for (unsigned int i = 0; i < meshes.size(); i++)
//		meshes[i].SetupInstanceData(dataBuffer, location, size, offset);
//}

void real::Model::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene * scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "\x1B[31mERROR::ASSIMP::" << import.GetErrorString() << std::endl << "\x1B[37m";
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void real::Model::loadModel(Primitives primitiveModel, Texture texture)
{

	std::vector<Vertex> vertices;

	switch (primitiveModel)
	{
		case Primitives::PRIMITIVE_PLANE:
		{
			float verts[] = {
				// positions          // Normals           // texture coords
				 1.f, -1.f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,   // bottom right
				 1.f,  1.f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f,   // top right
				-1.f, -1.f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left

				-1.f,  1.f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,   // top left 
				-1.f, -1.f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
				 1.f,  1.f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 1.0f    // top right
			};

			for (int i = 0; i < 6; i++)
			{
				int vertI = i * 8;

				Vertex vertex;

				glm::vec3 pos;
				pos.x = verts[vertI];
				pos.y = verts[vertI + 1];
				pos.z = verts[vertI + 2];

				glm::vec3 normal;
				normal.x = verts[vertI + 3];
				normal.y = verts[vertI + 4];
				normal.z = verts[vertI + 5];

				glm::vec2 texCoord;
				texCoord.x = verts[vertI + 6];
				texCoord.y = verts[vertI + 7];

				vertex.Position = pos;
				vertex.Normal = normal;
				vertex.TexCoords = texCoord;

				vertices.push_back(vertex);
			}
			break;
		}
		case Primitives::PRIMITIVE_CUBE:
		{
			float verts[] = {
				// positions          // normals           // texture coords
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
				-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,

				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
				-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
				-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
				-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

				 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
				 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
				 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,

				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
				 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
				 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
				-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
				-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
				 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
				 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
				-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
				-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f
			};

			for (int i = 0; i < 36; i++)
			{
				int vertI = i * 8;

				Vertex vertex;

				glm::vec3 pos;
				pos.x = verts[vertI];
				pos.y = verts[vertI + 1];
				pos.z = verts[vertI + 2];

				glm::vec3 normal;
				normal.x = verts[vertI + 3];
				normal.y = verts[vertI + 4];
				normal.z = verts[vertI + 5];

				glm::vec2 texCoord;
				texCoord.x = verts[vertI + 6];
				texCoord.y = verts[vertI + 7];

				vertex.Position = pos;
				vertex.Normal = normal;
				vertex.TexCoords = texCoord;

				vertices.push_back(vertex);
			}
			break;
		}
		default:
			break;
	}

	Material material;
	material.textures.push_back(texture);
	Mesh* mesh = new Mesh(vertices, std::vector<unsigned int>(), material);

	meshes.push_back(mesh);

}


//void Model::LoadCubeMap()
//{
//	Texture cubemapText;
//	cubemapText.id = cubemap.ID;
//	cubemapText.type = "texture_cubemap";
//	for (Mesh& mesh : meshes)
//	{
//		//Remove old cubemap
//		int i = 0;
//		for (Texture text : mesh.textures)
//		{
//			if (text.type == "texture_cubemap")
//			{
//				mesh.textures.erase(mesh.textures.begin()+i);
//			}
//			i++;
//		}
//
//		//Add new one
//		mesh.textures.push_back(cubemapText);
//	}
//}


void real::Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

real::Mesh* real::Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	bool opaque = true;

	//Process vertices
	glm::vec3 totalPosition = glm::vec3(0);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		
		SetVertexBoneDataToDefault(vertex);

		// process positions
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		totalPosition += vector;
		// Normals
		if (mesh->mNormals != nullptr)
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;
		}
		else
		{
			vertex.Normal = glm::vec3(0, 0, 0);
		}
		//Texture coords
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		//(bi)tangents
		if (mesh->mTangents != nullptr)
		{
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;

			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}


		//Push to list
		vertices.push_back(vertex);
	}
	totalPosition /= static_cast<float>(mesh->mNumVertices);
	// process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	Material meshMaterial;

	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
			aiTextureType_DIFFUSE, TEXTURE_TYPE_DIFFUSE);

		if (diffuseMaps.size() <= 0)
		{
			//Load missing texture
			meshMaterial.textures.push_back(m_textureManager->GetTexture("../Common/Assets/missing.jpg",real::TEXTURE_TYPE_DIFFUSE));
		}
		meshMaterial.textures.insert(meshMaterial.textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Texture> specularMaps = loadMaterialTextures(material,
			aiTextureType_SPECULAR, TEXTURE_TYPE_SPECULAR);
		meshMaterial.textures.insert(meshMaterial.textures.end(), specularMaps.begin(), specularMaps.end());

		//Should be textureType normal not height but obj exports it incorrectly in our case we dont have heightmaps so its no issue
		std::vector<Texture> normalMaps = loadMaterialTextures(material,
			aiTextureType_HEIGHT, TEXTURE_TYPE_NORMAL);
		meshMaterial.textures.insert(meshMaterial.textures.end(), normalMaps.begin(), normalMaps.end());

		std::vector<Texture> emissionMaps = loadMaterialTextures(material,
			aiTextureType_EMISSIVE, TEXTURE_TYPE_EMISSIVE);
		meshMaterial.textures.insert(meshMaterial.textures.end(), emissionMaps.begin(), emissionMaps.end());
		std::vector<Texture> alphaMaps = loadMaterialTextures(material,
			aiTextureType_OPACITY, TEXTURE_TYPE_ALPHA);
		if (alphaMaps.size() > 0) 
			opaque = false;
		meshMaterial.textures.insert(meshMaterial.textures.end(), alphaMaps.begin(), alphaMaps.end());
	}

	ExtractBoneWeightForVertices(vertices,mesh,scene);
	Mesh* createdMesh = new Mesh(vertices, indices, meshMaterial,totalPosition);
	createdMesh->opaque = opaque;
	return createdMesh;
}

std::vector<real::Texture> real::Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, E_BASE_TEXTURE_TYPES texture_type)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string filepath = directory + '/' + str.C_Str();
		Texture texture = m_textureManager->GetTexture(filepath, texture_type);
		textures.push_back(texture);
	}
	return textures;
}

void real::Model::ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene*)
{
	for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
	{
		int boneID = -1;
		std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
		if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
		{
			BoneInfo newBoneInfo;
			newBoneInfo.id = m_BoneCounter;
			aiMatrix4x4 aiOffsetMatrix = mesh->mBones[boneIndex]->mOffsetMatrix;
			glm::mat4 offsetMatrix;
			offsetMatrix[0][0] = aiOffsetMatrix.a1; offsetMatrix[1][0] = aiOffsetMatrix.a2; offsetMatrix[2][0] = aiOffsetMatrix.a3; offsetMatrix[3][0] = aiOffsetMatrix.a4;
			offsetMatrix[0][1] = aiOffsetMatrix.b1; offsetMatrix[1][1] = aiOffsetMatrix.b2; offsetMatrix[2][1] = aiOffsetMatrix.b3; offsetMatrix[3][1] = aiOffsetMatrix.b4;
			offsetMatrix[0][2] = aiOffsetMatrix.c1; offsetMatrix[1][2] = aiOffsetMatrix.c2; offsetMatrix[2][2] = aiOffsetMatrix.c3; offsetMatrix[3][2] = aiOffsetMatrix.c4;
			offsetMatrix[0][3] = aiOffsetMatrix.d1; offsetMatrix[1][3] = aiOffsetMatrix.d2; offsetMatrix[2][3] = aiOffsetMatrix.d3; offsetMatrix[3][3] = aiOffsetMatrix.d4;
			offsetMatrix = glm::rotate(offsetMatrix, glm::radians(90.f), glm::vec3(1, 0, 0)); // Mixamo exports its animations 90deg rotated so we need to rotate it back
			newBoneInfo.offset = offsetMatrix;

			m_BoneInfoMap[boneName] = newBoneInfo;
			boneID = m_BoneCounter;
			m_BoneCounter++;
		}
		else
		{
			boneID = m_BoneInfoMap[boneName].id;
		}
		assert(boneID != -1);
		auto weights = mesh->mBones[boneIndex]->mWeights;
		int numWeights = mesh->mBones[boneIndex]->mNumWeights;

		for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
		{
			int vertexId = weights[weightIndex].mVertexId;
			float weight = weights[weightIndex].mWeight;
			assert(vertexId <= vertices.size());
			SetVertexBoneData(vertices[vertexId], boneID, weight);
		}
	}
}