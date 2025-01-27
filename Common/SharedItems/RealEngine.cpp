#include "precomp.h"
#include "RealEngine.h"


#include "btBulletDynamicsCommon.h"


real::RealEngine::RealEngine()
{

}
real::RealEngine::~RealEngine()
{
	delete m_loadedScene;
	delete m_btDynamicsWorld;
	delete m_btCollisionConfig;
	delete m_btDispatcher;
	delete m_btBroadPhase;
	delete m_btSolver;
}

void real::RealEngine::Init()
{
	printf("\x1B[35m");
	printf(R"(
Initializing:
__________              .__  ___________              .__               
\______   \ ____ _____  |  | \_   _____/ ____    ____ |__| ____   ____  
 |       _// __ \\__  \ |  |  |    __)_ /    \  / ___\|  |/    \_/ __ \ 
 |    |   \  ___/ / __ \|  |__|        \   |  \/ /_/  >  |   |  \  ___/ 
 |____|_  /\___  >____  /____/_______  /___|  /\___  /|__|___|  /\___  >
        \/     \/     \/             \/     \//_____/         \/     \/ 
By Jack Tollenaar
)");
	printf("\x1B[37m\n");

	//shaderManager.Init(); //don't have init yet
	//textureManager.Init(); 
	m_modelManager.Init(m_textureManager);

	//Physics



	///collision configuration contains default setup for memory, collision setup
	m_btCollisionConfig = new btDefaultCollisionConfiguration();
	//m_collisionConfiguration->setConvexConvexMultipointIterations();

	m_btDispatcher = new btCollisionDispatcher(m_btCollisionConfig);

	m_btBroadPhase = new btDbvtBroadphase();

	///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
	btSequentialImpulseConstraintSolver* sol = new btSequentialImpulseConstraintSolver;
	m_btSolver = sol;

	m_btDynamicsWorld = new btDiscreteDynamicsWorld(m_btDispatcher,m_btBroadPhase,m_btSolver,m_btCollisionConfig);
	m_btDynamicsWorld->setGravity(btVector3(0, -10, 0));

	m_debugDrawer = new BtDebugDrawer();
	m_debugDrawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints | btIDebugDraw::DBG_DrawAabb);
	m_debugDrawer->Init(m_shaderManager);
	m_btDynamicsWorld->setDebugDrawer(m_debugDrawer);
	
	m_loadedScene = new Scene();
	m_loadedScene->Init(m_shaderManager, *m_btDynamicsWorld, *m_debugDrawer);


}

void real::RealEngine::Tick()
{
	m_audioManager.Tick();
}

void real::RealEngine::StepPhysics(float _deltaTime)
{
	m_btDynamicsWorld->stepSimulation(_deltaTime, 10, 1.0f / PHYSICS_TICKRATE);

	//Check collisions
	int numManifolds = m_btDynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++)
	{
		if (i > m_btDynamicsWorld->getDispatcher()->getNumManifolds()) break; // For some reason this can change randomly so make sure its correct
		btPersistentManifold* contactManifold = m_btDynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = static_cast<const btCollisionObject*>(contactManifold->getBody0());
		const btCollisionObject* obB = static_cast<const btCollisionObject*>(contactManifold->getBody1());

		for (ICollisionCallback* subscriber : m_collisionCallBackSubscribers)
		{
			if (obA == &subscriber->GetBody())
			{
				subscriber->OnCollision(*static_cast<GameObject*>(obB->getUserPointer()));
			}
			else if (obB == &subscriber->GetBody())
			{
				subscriber->OnCollision(*static_cast<GameObject*>(obA->getUserPointer()));
			}
		}

	}


}

void real::RealEngine::PhysicsDebugDraw()
{
	//Calculate lineswww
	if (m_btDynamicsWorld && m_btDynamicsWorld->getDebugDrawer())
	{
		m_debugDrawer->SetViewProjection(m_loadedScene->GetMainCamera().GetViewProjection());
		m_btDynamicsWorld->debugDrawWorld();
	}
	//Display
	m_debugDrawer->Draw();
}

real::GameObject& real::RealEngine::InstantiateObject(real::GameObject& _object)
{
	m_loadedScene->GetObjectManager().AddObject(_object);
	return _object;
}

real::GameObject& real::RealEngine::InstantiateObject(Model& _model)
{
	GameObject& newObject = *new GameObject();
	newObject.Init(_model);
	return InstantiateObject(newObject);
}

real::GameObject& real::RealEngine::InstantiateObject(std::string _modelPath, Shader& _shader)
{
	return InstantiateObject(m_modelManager.GetModel(_modelPath.c_str(),_shader));
}

void real::RealEngine::SubscribeToCollisionCallback(ICollisionCallback& _subscriber)
{
	m_collisionCallBackSubscribers.push_back(&_subscriber);
}

void real::RealEngine::UnSubscribeToCollisionCallback(ICollisionCallback& _subscriber)
{
	auto found = std::find(m_collisionCallBackSubscribers.begin(), m_collisionCallBackSubscribers.end(), &_subscriber);
	if (found != m_collisionCallBackSubscribers.end())
	{
		m_collisionCallBackSubscribers.erase(found);
	}
}