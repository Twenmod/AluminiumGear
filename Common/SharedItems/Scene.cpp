#include "precomp.h"
#include "Scene.h"
#include "Camera.h"
#include "ShaderManager.h"

#include "TextureManager.h"
#include "Model.hpp"
#include "NavMesh.h"
#include "ParticleSystem.h"

real::Scene::~Scene()
{
	delete m_navMesh;
	for (int i = 0; i < m_particles.size(); i++)
	{
		delete m_particles[i];
	}
}

void real::Scene::Init(ShaderManager& _shaderManager, btDynamicsWorld& _physicsWorld, BtDebugDrawer& _debugDrawer)
{
	m_objectManager.Init(_shaderManager, _physicsWorld, _debugDrawer);
	m_shaderManager = &_shaderManager;
}

void real::Scene::Tick(float _deltaTime)
{
	m_objectManager.Tick(_deltaTime);

	for (int i = 0; i < m_particles.size(); i++)
	{
		m_particles[i]->Tick(_deltaTime);
	}

}

void real::Scene::Draw(bool _wireframe)
{
	m_objectManager.Draw(*m_mainCamera, _wireframe);

	for (int i = 0; i < m_particles.size(); i++)
	{
		m_particles[i]->Draw();
	}

	//glm::mat4 skyView = glm::mat4(glm::mat3(GetMainCamera().GetView()));
	//real::Shader& skyShader = *shaderManager->GetShader("Skybox");
	//skyShader.use();
	//skyShader.setMat4("VP", GetMainCamera().GetProjection() * skyView);
	//glCullFace(GL_FRONT);
	//if (skybox != nullptr)
	//	skybox->Draw(wireframe);
	//glCullFace(GL_BACK);

}

void real::Scene::DeleteParticleSystem(ParticleSystem& _particleSystem)
{
	auto found = std::find(m_particles.begin(), m_particles.end(), &_particleSystem);
	if (found != m_particles.end())
	{
		delete *found;
		m_particles.erase(found);
	}
}
