#pragma once

class btDynamicsWorld;


namespace real
{
	class GameObject;
	class Shader;
	class Light;
	class ShaderManager;
	class Camera;
	class ReflectionProbe;
	class BtDebugDrawer;
	class GibbedObjectsSpawner;

	class ObjectManager
	{
	public:
		ObjectManager();
		~ObjectManager();
		void Init(ShaderManager& shaderManager, btDynamicsWorld& btPhysicsWorld, BtDebugDrawer& debugDrawer);
		void Tick(float deltaTime);
		void Draw(Camera& mainCamera, bool wireframe = false, bool isReflection = false);
		void AddObject(GameObject& objectToAdd);
		void RemoveObject(GameObject& objectToRemove);
		void AddReflectionProbe(ReflectionProbe& probe) { m_reflectionProbes.push_back(&probe); }
		void AddGibObjects(GibbedObjectsSpawner& gibSpawner) { m_gibbedObjects.push_back(&gibSpawner); }
		std::vector<ReflectionProbe*> GetSelectedReflectionProbes() { return m_selectedReflectionProbes; }
		std::array<GameObject*, MAX_OBJECTS> GetObjects() { return objectPool; }
		unsigned int GetObjectPoolSize() const { return m_currentObjectPoolSize; }
	private:
		std::array<GameObject*, MAX_OBJECTS> objectPool;
		std::vector<Light*> m_lights;
		std::vector<ReflectionProbe*> m_reflectionProbes;
		std::vector<ReflectionProbe*> m_selectedReflectionProbes;
		std::vector<GibbedObjectsSpawner*> m_gibbedObjects;
		unsigned int m_currentObjectPoolSize{ 0 };
		ShaderManager* m_shaderManager;
		BtDebugDrawer* m_debugDrawer;
		btDynamicsWorld* m_btPhysicsWorld;
	};
}