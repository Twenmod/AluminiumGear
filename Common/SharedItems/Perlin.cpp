#include "precomp.h"
#include "Perlin.h"

real::Perlin::Perlin()
{
	CreatePermutationTable();
}

float real::Perlin::SamplePerlin(glm::vec2 _position, float _frequency)
{

	//Inputs
	float x = _position.x * _frequency;
	float y = _position.y * _frequency;

	int X = static_cast<int>(floor(x)) % 256;
	int Y = static_cast<int>(floor(y)) % 256;
	//Position inside the gridbox
	float xf = x - floor(x);
	float yf = y - floor(y);

	glm::vec2 topRight = glm::vec2(xf - 1.0, yf - 1.0);
	glm::vec2 topLeft = glm::vec2(xf, yf - 1.0);
	glm::vec2 bottomRight = glm::vec2(xf - 1.0, yf);
	glm::vec2 bottomLeft = glm::vec2(xf, yf);

	// Select a value in the array for each of the 4 corners

	int valueTopRight = permutationTable[permutationTable[X + 1] + Y + 1];
	int valueTopLeft = permutationTable[permutationTable[X] + Y + 1];
	int valueBottomRight = permutationTable[permutationTable[X + 1] + Y];
	int valueBottomLeft = permutationTable[permutationTable[X] + Y];

	float dotTopRight = dot(topRight, GetConstantVector(valueTopRight));
	float dotTopLeft = dot(topLeft, GetConstantVector(valueTopLeft));
	float dotBottomRight = dot(bottomRight, GetConstantVector(valueBottomRight));
	float dotBottomLeft = dot(bottomLeft, GetConstantVector(valueBottomLeft));

	float u = Fade(xf);
	float v = Fade(yf);

	
	float lerpTop = glm::mix(dotTopLeft, dotTopRight, u);
	float lerpBottom = glm::mix(dotBottomLeft, dotBottomRight, u);
	float result = glm::mix(lerpBottom, lerpTop, v);

	//Change into [0-1]
	result = (result + 1) / 2.f;
	return result;
}

glm::vec2 real::Perlin::GetConstantVector(int _value)
{
	// v is the value from the permutation table
	int h = _value & 3;
	if (h == 0)
		return glm::vec2(1.0, 1.0);
	else if (h == 1)
		return glm::vec2(-1.0, 1.0);
	else if (h == 2)
		return glm::vec2(-1.0, -1.0);
	else
		return glm::vec2(1.0, -1.0);
}

float real::Perlin::Fade(float _time)
{
	return ((6 * _time - 15) * _time + 10) * _time * _time * _time;
}

void real::Perlin::CreatePermutationTable()
{
	//Create permutation table
	for (int i = 0; i < permutationSize; i++)
	{
		permutationTable[i] = i;
	}
	//Randomly shuffle the table
	for (int i = permutationSize - 1; i > 0; i--)
	{
		int swapIndex = static_cast<int>(round((rand() / static_cast<float>(RAND_MAX)) * (i - 1)));
		int old = permutationTable[i];
		permutationTable[i] = permutationTable[swapIndex];
		permutationTable[swapIndex] = old;
	}
}
