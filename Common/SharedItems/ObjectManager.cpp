#include "precomp.h"
#include "ObjectManager.h"

#include "GameObject.h"
#include "Light.h"
#include "ShaderManager.h"
#include "Camera.h"           

#include "BodyMotionState.h"
#include "ReflectionProbe.h"
#include "BTDebugDrawer.h"
#include "Model.hpp"
#include "GibbedGameObject.h"


real::ObjectManager::ObjectManager()
{
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		objectPool[i] = nullptr;
	}
}

real::ObjectManager::~ObjectManager()
{
	for (size_t i = 0; i < m_currentObjectPoolSize; i++)
	{
		if (objectPool[i]->m_body != nullptr)
			m_btPhysicsWorld->removeRigidBody(objectPool[i]->m_body);
		delete objectPool[i];
	}
	for (size_t i = 0; i < m_reflectionProbes.size(); i++)
	{
		delete m_reflectionProbes[i];
	}
	for (size_t i = 0; i < m_gibbedObjects.size(); i++)
	{
		delete m_gibbedObjects[i];
	}
}

void real::ObjectManager::Init(ShaderManager& _shaderManager, btDynamicsWorld& _btPhysicsWorld, BtDebugDrawer& _debugDrawer)
{
	m_shaderManager = &_shaderManager;
	m_btPhysicsWorld = &_btPhysicsWorld;
	m_debugDrawer = &_debugDrawer;
}

void real::ObjectManager::Tick(float deltaTime)
{
	for (size_t i = 0; i < m_currentObjectPoolSize; i++)
	{
		if (objectPool[i] == nullptr)
		{
			if (i < m_currentObjectPoolSize)
			{
				if (objectPool[m_currentObjectPoolSize-1] != nullptr)
				{
					objectPool[i] = objectPool[m_currentObjectPoolSize-1];
					objectPool[m_currentObjectPoolSize-1] = nullptr;
				}
			}
			m_currentObjectPoolSize--;
			i--;
		}
		if (objectPool[i] != nullptr) objectPool[i]->Tick(deltaTime);
	}
}

void real::ObjectManager::Draw(Camera& mainCamera, bool wireframe, bool _isReflection)
{
#if REFLECTIONS
	ReflectionProbe* reflectProbe = nullptr;
	ReflectionProbe* secondReflectProbe = nullptr;
	if (m_reflectionProbes.size() > 0 && !_isReflection)
	{
		//Look for closest probe
		float dist = 999999;
		float secondDist = 99999;
		int secondClosest = -1;
		int closest = -1;
		for (int i = 0; i < m_reflectionProbes.size(); i++)
		{
			ReflectionProbe* probe = m_reflectionProbes[i];
			glm::vec3 halfSize = probe->GetSize() * 0.5f;
			m_debugDrawer->drawLightProbe(GlmVecToBtVec(probe->GetPosition()), GlmVecToBtVec(probe->GetSize()), btVector3(1, 1, 0));

			float distance = length(mainCamera.GetPosition() - probe->GetPosition());

			if (distance < dist)
			{
				secondDist = dist;
				secondClosest = closest;

				dist = distance;
				closest = i;
			}
			else if (distance < secondDist)
			{
				secondDist = distance;
				secondClosest = i;
			}
		}
		reflectProbe = m_reflectionProbes[closest];
		secondReflectProbe = m_reflectionProbes[secondClosest];

		m_selectedReflectionProbes.clear();
		m_selectedReflectionProbes.push_back(reflectProbe);
		m_selectedReflectionProbes.push_back(secondReflectProbe);

		m_debugDrawer->drawLightProbe(GlmVecToBtVec(reflectProbe->GetPosition()), GlmVecToBtVec(reflectProbe->GetSize()), btVector3(1, 0, 0));
		m_debugDrawer->drawLightProbe(GlmVecToBtVec(secondReflectProbe->GetPosition()), GlmVecToBtVec(secondReflectProbe->GetSize()), btVector3(1, 0, 1));

		//float l1 = glm::length(reflectProbe->GetPosition() - mainCamera.GetPosition());
		//float l2 = length(secondReflectProbe->GetPosition() - mainCamera.GetPosition());
		//float a = l1 / l2;
	}
#endif



	//Set lighting data
	const int shaderTextureAmount = 6; // Amount of textures used in the shader

	std::vector<real::Shader*> shaders = m_shaderManager->GetAllShaders();
	for (int i = 0; i < shaders.size(); i++)
	{
		shaders[i]->use();
		int amountOfDirLights = 0;
		int amountOfPointLights = 0;

		const int AMOUNTDIRLIGHTS = 2;
		const int AMOUNTPOINTLIGHTS = 16;
		for (int j = 0; j < AMOUNTDIRLIGHTS; j++)
		{
			shaders[i]->setVec3("dirLight[" + std::to_string(j) + "]" + ".diffuse", glm::vec3(0));
			shaders[i]->setVec3("dirLight[" + std::to_string(j) + "]" + ".specular", glm::vec3(0));
		}
		for (int j = 0; j < AMOUNTPOINTLIGHTS; j++)
		{
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "]" + ".diffuse", glm::vec3(0));
			shaders[i]->setVec3("pointLights[" + std::to_string(j) + "]" + ".specular", glm::vec3(0));
			shaders[i]->setFloat("pointLights[" + std::to_string(j) + "]" + ".constant", 1.f);
		}


		for (Light* light : m_lights)
		{
			m_debugDrawer->drawLight(*light);
			switch (light->GetType())
			{
				case E_lightTypes::directionalLight:
					shaders[i]->setVec3("dirLight[" + std::to_string(amountOfDirLights) + "]" + ".direction", light->GetDirection());
					shaders[i]->setVec3("dirLight[" + std::to_string(amountOfDirLights) + "]" + ".diffuse", light->GetColor());
					shaders[i]->setVec3("dirLight[" + std::to_string(amountOfDirLights) + "]" + ".specular", light->GetSpecular());
					amountOfDirLights++;
					break;
				case E_lightTypes::pointLight:
					//cull if out of view
					glm::vec3 lightDir = light->GetPosition() - mainCamera.GetPosition();
					float lightDot = glm::dot(glm::normalize(lightDir), mainCamera.GetForward());
					float lightDist2 = dot(lightDir, lightDir);
					if (lightDist2 > LIGHT_CULL_MAX) break; // Cull if far
					if (lightDist2 > LIGHT_CULL_MIN // Cull if far enough and out of frustrum
					    && lightDot < LIGHT_CULL_DOT_HARD
					) break;

					float scalar = 1;
					if (lightDist2 < LIGHT_CULL_DOT_SOFT)
						scalar = std::max(std::min((((lightDot)-(LIGHT_CULL_DOT_HARD)) / (LIGHT_CULL_DOT_SOFT - LIGHT_CULL_DOT_HARD)), 1.f), 0.f);

					shaders[i]->setVec3("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".position", light->GetPosition()); 
					shaders[i]->setVec3("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".diffuse", light->GetColor()*scalar); 
					shaders[i]->setVec3("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".specular", light->GetSpecular());
					shaders[i]->setFloat("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".constant", light->GetConstant());
#ifdef WINDOWS_BUILD
					shaders[i]->setFloat("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".linear", light->GetLinear());
#else
					shaders[i]->setFloat("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".linear", sqrt(light->GetLinear()));
#endif
					shaders[i]->setFloat("pointLights[" + std::to_string(amountOfPointLights) + "]" + ".quadratic", light->GetQuadratic());
					amountOfPointLights++;
					break;
			}
		}
		shaders[i]->setInt("amountOfDirLights", amountOfDirLights);
		shaders[i]->setInt("amountOfPointLights", amountOfPointLights);
		shaders[i]->setVec3("ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		glm::vec3 camPos = mainCamera.GetPosition();
		shaders[i]->setVec3("viewPos", camPos);
#if REFLECTIONS
		if (reflectProbe != nullptr && !_isReflection)
		{
			shaders[i]->setBool("reflective", true);
			shaders[i]->setVec3("reflectProbePos", reflectProbe->GetPosition());
			shaders[i]->setVec3("probeHalfSize", reflectProbe->GetSize()*0.5f);
			shaders[i]->setVec3("reflectProbePos2", secondReflectProbe->GetPosition());
			shaders[i]->setVec3("probeHalfSize2", secondReflectProbe->GetSize()*0.5f);
		}
		else
		{
			shaders[i]->setBool("reflective", false);
		}
#else
		m_shaders[i]->setBool("reflective", false);
#endif

	}

	std::vector<ReflectionProbe*> activeReflectionProbes;
#if REFLECTIONS
	if (reflectProbe != nullptr && !_isReflection)
	{
		glActiveTexture(GL_TEXTURE0+ shaderTextureAmount);
		glBindTexture(GL_TEXTURE_CUBE_MAP, reflectProbe->GetCubemap().id);
		glActiveTexture(GL_TEXTURE1+ shaderTextureAmount);
		glBindTexture(GL_TEXTURE_CUBE_MAP, secondReflectProbe->GetCubemap().id);

		activeReflectionProbes.push_back(reflectProbe);
		activeReflectionProbes.push_back(secondReflectProbe);
	}
#endif

	glm::mat4 viewProjection = mainCamera.GetViewProjection();

	std::vector<std::pair<GameObject*, Mesh*>> opaqueList;
	std::vector<std::pair<GameObject*, Mesh*>> transparentList;

	//Objects associated to the meshes

	for (size_t i = 0; i < m_currentObjectPoolSize; i++)
	{
		if (objectPool[i] != nullptr && objectPool[i]->GetModel() != nullptr)
		{
			for (Mesh* mesh : objectPool[i]->GetModel()->GetMeshes())
			{
				if (mesh->opaque)
					opaqueList.push_back(std::pair<GameObject*, Mesh*>(objectPool[i], mesh));
				else
					transparentList.push_back(std::pair<GameObject*, Mesh*>(objectPool[i], mesh));

			}
		}
	}

	//Samples the first vertice for position since meshes part of the same model have same position
	std::sort(transparentList.begin(), transparentList.end(), // Sort transparent list back to front
		[&mainCamera](const std::pair<GameObject*, Mesh*>& a, const std::pair<GameObject*, Mesh*>& b)
		{
			return glm::length(a.second->vertices[0].Position - mainCamera.GetPosition()) > glm::length(b.second->vertices[0].Position - mainCamera.GetPosition());
		}
	);
	std::sort(opaqueList.begin(), opaqueList.end(), // Sort opaque objects front to back
		[&mainCamera](const std::pair<GameObject*, Mesh*>& a, const std::pair<GameObject*, Mesh*>& b)
		{
			return glm::length(a.second->vertices[0].Position - mainCamera.GetPosition()) < glm::length(b.second->vertices[0].Position - mainCamera.GetPosition());
		}
	);

	for (size_t i = 0; i < opaqueList.size(); i++)
	{
		opaqueList[i].first->SetupDraw(viewProjection, activeReflectionProbes, wireframe);
		opaqueList[i].second->Draw(const_cast<Shader&>(opaqueList[i].first->GetModel()->GetShader()), activeReflectionProbes, wireframe);
	}

	for (size_t i = 0; i < transparentList.size(); i++)
	{
		transparentList[i].first->SetupDraw(viewProjection, activeReflectionProbes, wireframe);
		transparentList[i].second->Draw(const_cast<Shader&>(transparentList[i].first->GetModel()->GetShader()), activeReflectionProbes, wireframe);
	}
}

void real::ObjectManager::AddObject(GameObject& objectToAdd)
{
	objectPool[m_currentObjectPoolSize++] = &objectToAdd;
	if (Light* light = dynamic_cast<Light*>(&objectToAdd))
	{
		m_lights.push_back(light);
	}
}

void real::ObjectManager::RemoveObject(GameObject& objectToRemove)
{
	for (size_t i = 0; i < m_currentObjectPoolSize; i++)
	{
		if (objectPool[i] == &objectToRemove)
		{
			if (Light* light = dynamic_cast<Light*>(&objectToRemove))
			{
				m_lights.erase(std::find(m_lights.begin(), m_lights.end(), light));
			}
			if (objectPool[i]->m_body != nullptr)
				m_btPhysicsWorld->removeRigidBody(objectPool[i]->m_body);
			delete objectPool[i];
			objectPool[i] = nullptr;
		}
	}
}