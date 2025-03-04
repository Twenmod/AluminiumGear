#pragma once

constexpr unsigned int perlinSize = 256;

struct Vertex
{
	glm::vec3 position; 
	glm::vec3 normal;    
	glm::vec2 texCoords; 
};

class Water : public real::GameObject
{
public:
	Water(glm::vec2 planeSize = glm::vec2(1.0f));
	~Water();
	void Init(real::Shader& shader, real::Perlin& perlinNoise);
	void Tick(float deltaTime);
	void SetupDraw(glm::mat4 viewProjection, std::vector<real::ReflectionProbe*> activeReflectionProbes = std::vector<real::ReflectionProbe*>(), bool wireframe = false) override;
	void Draw();
private:
	// Function to generate plane vertices generated by CHATGPT
	std::vector<Vertex> GeneratePlaneWithSubdivisions(int subdivisions, float depth, const std::vector<float>& heights);
	std::vector<unsigned int> GeneratePlaneIndices(int subdivisions);

	const unsigned int m_subdivisions{ 20 };
	const float m_waterHeight{ 1.5f };
	glm::vec2 m_heightMapPos{ glm::vec2(0) };
	float m_waterSpeed{ 0.005f };
	float m_textureSpeed{ 0.001f };
	const glm::vec2 m_planeSize{ glm::vec2(1.0f) };
	const float m_scale{ 0.07f };
	std::vector<float> m_heightPerlin{ std::vector<float>(perlinSize * perlinSize) };
	std::vector<unsigned int> m_indices;
	float m_shaderTimer{ 0.f };
	GLuint m_VAO, m_VBO, m_EBO;
	GLuint m_perlinTexture;
	real::Shader* m_shader{nullptr};
};