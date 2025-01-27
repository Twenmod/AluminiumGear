#include "precomp.h"
#include "ShaderManager.h"

#include "ComputeShader.h"

real::ShaderManager::ShaderManager()
{

};

real::ShaderManager::~ShaderManager()
{
	for (auto& pair : m_shaders)
	{
		delete pair.second;
	}
}

real::Shader* real::ShaderManager::GetShader(std::string _shaderName, E_SHADER_TYPE _shaderType)
{

	Shader* foundShader;
	std::unordered_map<std::string, Shader*>::iterator found = m_shaders.find(_shaderName);
	if (found != m_shaders.end())
	{
		foundShader = m_shaders.at(_shaderName);
	}
	else
	{
		if (_shaderType == E_SHADER_TYPE::FRAG_VERT_SHADER)
		{
			std::string vertexShaderPath = SHADER_FOLDER + _shaderName + ".vert";
			std::string fragmentShaderPath = SHADER_FOLDER + _shaderName + ".frag";

			Shader* loadedShader = new Shader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());

			m_shaders.emplace(_shaderName, loadedShader);

			printf("\x1B[32mLoaded frag+vert shader at: %s \n\x1B[37m", vertexShaderPath.c_str());

			foundShader = m_shaders.at(_shaderName);
		}
		else // Compute shader
		{
			std::string computeShaderPath = SHADER_FOLDER + _shaderName + ".glsl";

			Shader* loadedShader = new ComputeShader(computeShaderPath.c_str());

			m_shaders.emplace(_shaderName, loadedShader);

			printf("\x1B[32mLoaded compute shader at: %s \n\x1B[37m", computeShaderPath.c_str());

			foundShader = m_shaders.at(_shaderName);

		}
	}
	return foundShader;

};

std::vector<real::Shader*> real::ShaderManager::GetAllShaders()
{
	std::vector<real::Shader*> returnList;
	if (m_shaders.size() == 0) return returnList;
	returnList.reserve(m_shaders.size());
	for (auto& pair : m_shaders)
	{
		returnList.push_back(pair.second);
	}
	return returnList;
}