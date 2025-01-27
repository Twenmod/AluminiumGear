#pragma once

namespace real
{
	class Scene;
	class GameObject;
	class Model;
	class ObjectManager;
	class TextureManager;

	class GibbedObjectsSpawner
	{
	public:
		GibbedObjectsSpawner() = default;
		~GibbedObjectsSpawner();
		void Init(real::Model& modelToSplit, real::TextureManager& _textureManager, float mass = 10.f, glm::vec3 spawnPosition = glm::vec3(0), glm::quat _spawnRotation = glm::identity<glm::quat>());
		void AddToScene(real::Scene& scene);
		void AddToScene(real::ObjectManager& objectManager);
		void AddToDynamicsWorld(btDynamicsWorld& dynamicsWorld);
		std::vector<real::GameObject*> GetObjects() const { return m_objects; }
	private:
		std::vector<real::GameObject*> m_objects;
		std::vector<real::Model*> m_createdSubModels;
	};
}
