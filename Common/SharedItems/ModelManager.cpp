#include "precomp.h"
#include "TextureManager.h"
#include "ModelManager.h"

#include "Shader.h"

real::ModelManager::ModelManager()
{

}

real::ModelManager::~ModelManager()
{
	for (std::pair<std::string, Model*> modelPair : m_models)
	{
		delete modelPair.second;
	}
}

void real::ModelManager::Init(TextureManager& _textureManager)
{
	m_textureManager = &_textureManager;
}

real::Model& real::ModelManager::GetModel(const char* _path, Shader& _shader)
{
	const char* path = _path + _shader.m_ID;

	Model* foundModel;
	std::unordered_map<std::string, Model*>::iterator found = m_models.find(path);
	if (found == m_models.end())
	{
		//Not loaded yet so load
		foundModel = new Model();
		foundModel->Init(m_textureManager, _path, _shader);

		//Move into models
		m_models.insert(std::pair<const char*, Model*>(path, std::move(foundModel)));
		//Delete temp version
		printf("\x1B[32mLoaded model: %s\n\x1B[37m", _path);
	}
	else
	{
		foundModel = m_models.at(path);
	}

	return *foundModel;
}

real::Model& real::ModelManager::GetModel(Primitives _primitiveModel, Texture& _texture, Shader& _shader)
{
	const char* path = "Primitive";
	if (_primitiveModel == Primitives::PRIMITIVE_PLANE) path = "Primitive::Plane";
	else if (_primitiveModel == Primitives::PRIMITIVE_CUBE) path = "Primitive::Cube";

	Model* foundModel;
	std::unordered_map<std::string, Model*>::iterator found = m_models.find(path);
	if (found == m_models.end())
	{
		//Not loaded yet so load
		foundModel = new Model();
		foundModel->Init(m_textureManager, _primitiveModel, _texture, _shader);

		m_models.insert(std::pair<const char*, Model*>(path, foundModel));

		printf("\x1B[32mLoaded model: %s\x1B[37m", path);
	}
	else
	{
		foundModel = m_models.at(path);
	}

	return *foundModel;
}

