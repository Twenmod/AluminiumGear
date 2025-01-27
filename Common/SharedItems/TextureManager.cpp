#define STB_IMAGE_IMPLEMENTATION
#include "precomp.h"
#include "TextureManager.h"

real::TextureManager::TextureManager()
{
}

real::TextureManager::~TextureManager()
{
	for (int i = 0; i < textures.size(); i++)
	{
		glDeleteTextures(1, &textures[i].id);
	}
}

int real::TextureManager::LoadTexture(std::string _file, E_BASE_TEXTURE_TYPES _type, bool _flipVertically)
{
	stbi_set_flip_vertically_on_load(_flipVertically);

	//Make object
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture

	std::replace(_file.begin(), _file.end(), '\\', '/'); // Can possibly break shit but fixes most issues with fbx using windows style formatting just make sure there are no spaces ig


	int width, height, nrChannels;
	unsigned char* data = stbi_load(_file.c_str(), &width, &height, &nrChannels, 0);
	if (data)
	{
		if (nrChannels > 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "\x1B[31mFailed to load texture at path " << _file << std::endl << "\x1B[37m";
	}

	//Create struct
	Texture newTexture;
	newTexture.id = textureID;
	newTexture.type = _type;
	newTexture.path = _file;

	textures.push_back(newTexture);

	//Free stb
	stbi_image_free(data);

	return static_cast<int>(textures.size()) - 1; // New loaded texture is at end of the array
}

int real::TextureManager::LoadTexture(std::vector<std::string>& _facePaths, E_BASE_TEXTURE_TYPES _type, bool _flipVertically)
{
	stbi_set_flip_vertically_on_load(_flipVertically);

	//Make object
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < _facePaths.size(); i++)
	{
		unsigned char* data = stbi_load(_facePaths[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "\x1B[31mCubemap texture failed to load at path: " << _facePaths[i] << std::endl << "\x1B[37m";
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Create struct
	Texture newTexture;
	newTexture.id = textureID;
	newTexture.type = _type;
	newTexture.path = _facePaths[0];

	textures.push_back(newTexture);

	return static_cast<int>(textures.size()) - 1; // New loaded texture is at end of the array
}

real::Texture& real::TextureManager::GetTexture(std::string _path, E_BASE_TEXTURE_TYPES _type)
{
	int foundTexture = -1;

	for (int i = 0; i < textures.size(); i++)
	{
		if (std::strcmp(textures[i].path.c_str(), _path.c_str()) == 0)
		{
			foundTexture = i;
			break;
		}
	}

	if (foundTexture == -1) // Texture is not loaded yet so load it
	{
		foundTexture = LoadTexture(_path, _type, false);
		printf("\x1B[32mLoaded texture at: %s\n\x1B[37m", _path.c_str());

	}

	return textures[foundTexture];
}

real::Texture& real::TextureManager::GetTexture(std::vector<std::string> _facePaths, E_BASE_TEXTURE_TYPES _type)
{
	int foundTexture = -1;

	for (int i = 0; i < textures.size(); i++)
	{
		if (std::strcmp(textures[i].path.c_str(), _facePaths[0].c_str()) == 0)
		{
			foundTexture = i;
			break;
		}
	}

	if (foundTexture == -1) // Texture is not loaded yet so load it
	{
		foundTexture = LoadTexture(_facePaths, _type, false);
		printf("\x1B[32mLoaded texture at: %s\n\x1B[37m", _facePaths[0].c_str());
	}

	return textures[foundTexture];
}