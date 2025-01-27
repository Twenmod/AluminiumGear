#include "precomp.h"
#include "GibbedGameObject.h"
#include "Scene.h"
#include "TextureManager.h"
#include "Model.hpp"
#include "GameObject.h"

real::GibbedObjectsSpawner::~GibbedObjectsSpawner()
{
	for (int i = 0; i < m_createdSubModels.size(); i++)
	{
		delete m_createdSubModels[i];
	}
}

void real::GibbedObjectsSpawner::Init(real::Model& _modelToSplit, real::TextureManager& _textureManager, float _mass, glm::vec3 _spawnPosition, glm::quat _spawnRotation)
{
	std::vector<real::Mesh*> meshes = _modelToSplit.GetMeshes();


	for (size_t i = 0; i < meshes.size(); i++)
	{
		real::Model* model = new real::Model();
		
		real::Mesh* newMesh = new real::Mesh(*meshes[i]);
		model->Init(*newMesh,const_cast<real::Shader&>(_modelToSplit.GetShader()), &_textureManager);

		real::GameObject* object = new real::GameObject();
		object->Init(*model,real::E_GENERATECOLLISIONSHAPEMODE::GENERATE_HULL,_mass,_spawnPosition + meshes[i]->m_centerPosition,glm::vec3(1),_spawnRotation, -meshes[i]->m_centerPosition);
		object->SetModelOffset(-meshes[i]->m_centerPosition,true);
		m_objects.push_back(object);
		m_createdSubModels.push_back(model);
	}
}

void real::GibbedObjectsSpawner::AddToScene(real::Scene& _scene)
{
	AddToScene(_scene.GetObjectManager());
}

void real::GibbedObjectsSpawner::AddToScene(real::ObjectManager& _objectManager)
{
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		_objectManager.AddObject(*m_objects[i]);
	}
}

void real::GibbedObjectsSpawner::AddToDynamicsWorld(btDynamicsWorld& _dynamicsWorld)
{
	for (size_t i = 0; i < m_objects.size(); i++)
	{
		_dynamicsWorld.addRigidBody(m_objects[i]->m_body);
	}
}


