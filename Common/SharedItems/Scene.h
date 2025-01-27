#pragma once
#include "ObjectManager.h"

namespace real
{
	class ObjectManager;
	class ModelManager;
	class Shader;
	class Camera;
	class InputManager;
	class Model;
	class BtDebugDrawer;
	class NavMesh;
	class ParticleSystem;

	class Scene
	{
	public:
		Scene() {};
		~Scene();
		void Init(ShaderManager& shaderManager, btDynamicsWorld& physicsWorld, BtDebugDrawer& debugDrawer);
		void Tick(float deltaTime);
		void Draw(bool wireframe = false);
		ObjectManager& GetObjectManager() { return m_objectManager; }
		void SetMainCamera(Camera& newMainCam) { m_mainCamera = &newMainCam; }
		Camera& GetMainCamera() { return *m_mainCamera; }
		void ReadMapData(char file[], int out[ROOM_TILE_WIDTH][ROOM_TILE_HEIGTH]);
		void SetNavigationMesh(NavMesh& _navMesh) { m_navMesh = &_navMesh; }
		void AddParticleSystem(ParticleSystem& _particleSystem) { m_particles.push_back(&_particleSystem); }
		void DeleteParticleSystem(ParticleSystem& particleSystem);;

		NavMesh* GetNavigationMesh() { return m_navMesh; }

		real::Model* m_skybox;
			int m_sceneID{ 0 };
	protected:
		ObjectManager m_objectManager;
			Camera* m_mainCamera{ nullptr };
			ShaderManager* m_shaderManager{ nullptr };
			std::vector<ParticleSystem*> m_particles;
			NavMesh* m_navMesh{ nullptr };
	};
}