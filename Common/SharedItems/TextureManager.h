#pragma once
#include <unordered_map>

namespace real
{

	enum E_BASE_TEXTURE_TYPES
	{
		TEXTURE_TYPE_DIFFUSE,
		TEXTURE_TYPE_SPECULAR,
		TEXTURE_TYPE_CUBEMAP,
		TEXTURE_TYPE_NORMAL,
		TEXTURE_TYPE_EMISSIVE,
		TEXTURE_TYPE_ALPHA
	};

	struct Texture
	{
		unsigned int id;
		E_BASE_TEXTURE_TYPES type;
		std::string path;
	};


	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();
		Texture& GetTexture(std::string path, E_BASE_TEXTURE_TYPES type);
		Texture& GetTexture(std::vector<std::string> facePaths, E_BASE_TEXTURE_TYPES type);
	private:
		std::vector<Texture> textures;
		int LoadTexture(std::string file, E_BASE_TEXTURE_TYPES type = TEXTURE_TYPE_DIFFUSE, bool flipVertically = true);
		int LoadTexture(std::vector<std::string>& facePaths, E_BASE_TEXTURE_TYPES type = TEXTURE_TYPE_CUBEMAP, bool flipVertically = true);
	};

}