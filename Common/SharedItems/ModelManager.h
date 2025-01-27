#pragma once
#include <unordered_map>
#include "Model.hpp"

namespace real
{
	class TextureManager;

	class ModelManager
	{
	public:
		ModelManager();
		~ModelManager();
		void Init(TextureManager& textureManager);
		Model& GetModel(const char* path, Shader& shader);
		Model& GetModel(Primitives primitiveModel, Texture& texture, Shader& shader);
	private:
		TextureManager* m_textureManager;
		std::unordered_map<std::string, Model*> m_models;
	};
}