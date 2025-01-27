#include "RealEngine.h"
#include "WaterModel.h"

Water::Water(glm::vec2 _planeSize) :
	m_planeSize(_planeSize)
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glGenBuffers(1, &m_EBO);
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

	m_indices = GeneratePlaneIndices(m_subdivisions);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
		&m_indices[0], GL_STATIC_DRAW);
}

Water::~Water()
{
	glDeleteTextures(1, &m_perlinTexture);
}


void Water::Init(real::Shader& _shader, real::Perlin& _perlin)
{
	m_shader = &_shader;

	std::vector<unsigned char> perlinNoiseTexture(256 * 256 * 3);
	for (int i = 0; i < 256 * 256; i++)
	{
		int y = static_cast<int>(floor(i / 256));
		int x = i - y * 256;
		float perlin = _perlin.SamplePerlin(glm::vec2(x, y),0.356f);
		unsigned char value = static_cast<unsigned char>(perlin * 0xff);
		m_heightPerlin[i] = perlin;
		perlinNoiseTexture[i * 3 + 0] = value;  // R
		perlinNoiseTexture[i * 3 + 1] = value;  // G
		perlinNoiseTexture[i * 3 + 2] = value;  // B	
	}

	glGenTextures(1, &m_perlinTexture);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, perlinNoiseTexture.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);

	GameObject::Init();
}

void Water::Tick(float _deltaTime)
{
	m_heightMapPos += glm::vec2(1) * _deltaTime * m_waterSpeed;
	m_shaderTimer = static_cast<float>(fmod(m_shaderTimer + _deltaTime * m_textureSpeed,300)); // fmod to avoid it going too high and causing innacuracy
}

void Water::SetupDraw(glm::mat4 _viewProjection, std::vector<real::ReflectionProbe*> _activeReflectionProbes, bool)
{
	//Calculate model
	glm::mat4 transform = glm::mat4(1);
	btTransform trans = GetGraphicTransform();
	btVector3 origin = trans.getOrigin();
	glm::vec3 objectposition(origin.getX(), origin.getY(), origin.getZ());

	transform = glm::translate(transform, objectposition + m_modelOffset);
	transform *= glm::mat4_cast(GetRotation());
	transform = glm::scale(transform, GetScale());
	//Set shader info
	m_shader->use();
	m_shader->setMat4("Model", transform);
	m_shader->setMat3("NormalMatrix", glm::mat3(glm::transpose(glm::inverse(transform))));
	m_shader->setMat4("VP", _viewProjection);

	if (_activeReflectionProbes.size() < 2)
	{
		m_shader->setInt("cubemap[" + std::to_string(0) + "]", CUBEMAPTEXTUREPOSITION + 0);
		m_shader->setInt("cubemap[" + std::to_string(1) + "]", CUBEMAPTEXTUREPOSITION + 1);
	}
	for (unsigned int i = 0; i < _activeReflectionProbes.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + CUBEMAPTEXTUREPOSITION + i);
		m_shader->setInt("cubemap[" + std::to_string(i) + "]", CUBEMAPTEXTUREPOSITION + i);
		glBindTexture(GL_TEXTURE_CUBE_MAP, _activeReflectionProbes[i]->GetCubemap().id);
	}

}

void Water::Draw()
{
	m_shader->use();

	m_shader->setFloat("timer", m_shaderTimer);

	for (int i = 0; i < 8; i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glActiveTexture(GL_TEXTURE0);
	m_shader->setInt("material.diffuse", 0);
	glBindTexture(GL_TEXTURE_2D, m_perlinTexture);



	//Create the vertices
	std::vector<Vertex> vertices = GeneratePlaneWithSubdivisions(m_subdivisions, m_waterHeight, m_heightPerlin);

	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);
	glDrawElements(GL_TRIANGLES, static_cast<int>(m_indices.size()), GL_UNSIGNED_INT, 0);

}

std::vector<Vertex> Water::GeneratePlaneWithSubdivisions(int subdivisions, float depth, const std::vector<float>& heights)
{
	int gridSize = subdivisions + 1;
	std::vector<Vertex> vertices;

	float dx = m_planeSize.x / subdivisions;  // Step size in the X direction
	float dz = m_planeSize.y / subdivisions;  // Step size in the Z direction

	for (int z = 0; z < gridSize; ++z)
	{
		for (int x = 0; x < gridSize; ++x)
		{
			float px = x * dx - m_planeSize.x / 2.0f;   // X position
			float pz = z * dz - m_planeSize.y / 2.0f;   // Z position

			float tx = static_cast<float>(x) / subdivisions;
			float tz = static_cast<float>(z) / subdivisions;

			//"fit" the texture inside
			float scaledX = tx * m_scale;
			float scaledZ = tz * m_scale;
			if (m_planeSize.x != m_planeSize.y)
			{
				scaledZ = scaledZ * m_planeSize.y / m_planeSize.x;
			}
			else if (m_planeSize.x > m_planeSize.y)
			{
				scaledX = scaledX * m_planeSize.x / m_planeSize.y;
			}



			//Heightmap sampling with interp added by me

			int heightGridSize = static_cast<int>(sqrt(heights.size()) - 1);


			//First noise
			float hPosX = static_cast<float>(fmod(scaledX + (m_heightMapPos.x), 1));
			float hPosZ = static_cast<float>(fmod(scaledZ + m_heightMapPos.y, 1));

			float gridX = hPosX * heightGridSize;
			int minX = static_cast<int>(floor(gridX));
			int maxX = static_cast<int>(ceil(gridX));
			float xAlpha = gridX - trunc(gridX);

			float gridZ = hPosZ * heightGridSize;
			int minZ = static_cast<int>(floor(gridZ));
			int maxZ = static_cast<int>(ceil(gridZ));
			float zAlpha = gridZ - trunc(gridZ);

			float topLeft = heights[minX + minZ * heightGridSize];
			float topRight = heights[maxX + minZ * heightGridSize];
			float botLeft = heights[minX + maxZ * heightGridSize];
			float botRight = heights[maxX + maxZ * heightGridSize];

			float py0 = glm::mix(topLeft, topRight, xAlpha);
			float py1 = glm::mix(botLeft, botRight, xAlpha);
			float pyfirst = glm::mix(py0, py1, zAlpha);


			//Second noise
			hPosX = static_cast<float>(fmod(scaledX - (m_heightMapPos.x) + 32.5, 1)); // Also adds random constant offset to make them less similar
			hPosZ = static_cast<float>(fmod(scaledZ - m_heightMapPos.y + 62.5, 1));

			gridX = hPosX * heightGridSize;
			minX = static_cast<int>(floor(gridX));
			maxX = static_cast<int>(ceil(gridX));
			xAlpha = gridX - trunc(gridX);

			gridZ = hPosZ * heightGridSize;
			minZ = static_cast<int>(floor(gridZ));
			maxZ = static_cast<int>(ceil(gridZ));
			zAlpha = gridZ - trunc(gridZ);

			topLeft = heights[minX + minZ * heightGridSize];
			topRight = heights[maxX + minZ * heightGridSize];
			botLeft = heights[minX + maxZ * heightGridSize];
			botRight = heights[maxX + maxZ * heightGridSize];

			py0 = glm::mix(topLeft, topRight, xAlpha);
			py1 = glm::mix(botLeft, botRight, xAlpha);
			float pysecond = glm::mix(py0, py1, zAlpha);

			float py = pyfirst * pysecond*depth;

			Vertex vert;
			vert.position = glm::vec3(px, py, pz);
			vert.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			vert.texCoords = glm::vec2(scaledX, scaledZ);

			vertices.push_back(vert);
		}
	}

	//calculate normals
	for (int z = 0; z < gridSize; ++z)
	{
		for (int x = 0; x < gridSize; ++x)
		{
			glm::vec3 currentPos = vertices[z * gridSize + x].position;

			//get neighbours
			glm::vec3 leftPos = (x > 0) ? vertices[z * gridSize + (x - 1)].position : currentPos;
			glm::vec3 rightPos = (x < gridSize - 1) ? vertices[z * gridSize + (x + 1)].position : currentPos;
			glm::vec3 upPos = (z > 0) ? vertices[(z - 1) * gridSize + x].position : currentPos;
			glm::vec3 downPos = (z < gridSize - 1) ? vertices[(z + 1) * gridSize + x].position : currentPos;

			//Get cross product
			glm::vec3 dxVec = rightPos - leftPos;
			glm::vec3 dzVec = downPos - upPos;
			glm::vec3 normal = glm::normalize(glm::cross(dzVec, dxVec));

			vertices[z * gridSize + x].normal = normal;
		}
	}


	return vertices;
}

std::vector<unsigned int> Water::GeneratePlaneIndices(int subdivisions)
{
	int gridSize = subdivisions + 1;
	std::vector<unsigned int> indices;

	for (int z = 0; z < subdivisions; ++z)
	{
		for (int x = 0; x < subdivisions; ++x)
		{
			int topLeft = z * gridSize + x;
			int topRight = topLeft + 1;
			int bottomLeft = (z + 1) * gridSize + x;
			int bottomRight = bottomLeft + 1;

			//First triangle (top-left, bottom-left, top-right)
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);
			indices.push_back(topRight);

			//Second triangle (top-right, bottom-left, bottom-right)
			indices.push_back(topRight);
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
	}

	return indices;
}
