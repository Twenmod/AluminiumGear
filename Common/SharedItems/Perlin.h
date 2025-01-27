#pragma once

//Implementation based on 
//https://rtouti.github.io/graphics/perlin-noise-algorithm


namespace real
{
	constexpr unsigned int permutationSize = 256;

	class Perlin
	{
	public:
		Perlin();
		float SamplePerlin(glm::vec2 pos, float frequency = 0.256f);
	private:
		glm::vec2 GetConstantVector(int value);
		float Fade(float time);
		void CreatePermutationTable();
		unsigned int permutationTable[permutationSize];
	};
}