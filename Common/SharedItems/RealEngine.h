#pragma once

#include "precomp.h"
#include "IInput.h"
#include "Input.h"

#include "AudioManager.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "ObjectManager.h"
#include "ShaderManager.h"
#include "Scene.h"
#include "Model.hpp"
#include "Shader.h"
#include "ComputeShader.h"
#include "GameObject.h"
#include "AnimatedGameObject.h"
#include "Camera.h"
#include "RealMath.h"
#include "NavMesh.h"
#include "UIManager.h"
#include "Perlin.h"
#include "ReflectionProbe.h"
#include "ParticleSystem.h"

#include "BTDebugDrawer.h"

#include "btBulletDynamicsCommon.h"
#include "ICollisionCallback.h"

namespace real
{
	class RealEngine
	{
	public:
		RealEngine();
		~RealEngine();
		void Init();
		void Tick();
		void StepPhysics(float deltaTime);
		void PhysicsDebugDraw();
		GameObject& InstantiateObject(GameObject& object);
		GameObject& InstantiateObject(Model& model);
		GameObject& InstantiateObject(std::string modelPath, Shader& shader);
		void SubscribeToCollisionCallback(ICollisionCallback& subscriber);
		void UnSubscribeToCollisionCallback(ICollisionCallback& subscriber);
		Scene* m_loadedScene{ nullptr };
		TextureManager m_textureManager;
		ModelManager m_modelManager;
		ShaderManager m_shaderManager;
		btDiscreteDynamicsWorld* m_btDynamicsWorld;
		BtDebugDrawer* m_debugDrawer;
		AudioManager m_audioManager;
		Perlin m_perlin;
	private:
		std::vector<ICollisionCallback*> m_collisionCallBackSubscribers;
		btDefaultCollisionConfiguration* m_btCollisionConfig;
		btCollisionDispatcher* m_btDispatcher;
		btDbvtBroadphase* m_btBroadPhase;
		btSequentialImpulseConstraintSolver* m_btSolver;
	};
}