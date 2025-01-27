#pragma once
#include "Common.h"
#include <unordered_map>
#include <string>
#include "Shader.h"

namespace real
{
	enum class E_SHADER_TYPE
	{
		FRAG_VERT_SHADER,
		COMPUTE_SHADER,
	};


	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();
		Shader* GetShader(std::string shaderName,E_SHADER_TYPE shaderType = E_SHADER_TYPE::FRAG_VERT_SHADER);
		std::vector<Shader*> GetAllShaders();
	private:
		std::unordered_map<std::string, Shader*> m_shaders;
	};
}