#pragma once
#include "TextureManager.h"

namespace real
{
	class ObjectManager;

	class ReflectionProbe
	{
	public:
		ReflectionProbe(glm::vec3 position, glm::vec3 size = glm::vec3(100,40,100), int resolution = 1024);
		~ReflectionProbe();
		/// <summary>
		/// Generate a cubemap from this location
		/// </summary>
		void Probe(ObjectManager& drawList);
		const Texture& GetCubemap() const { return m_cubemap; }
		glm::vec3 GetPosition() const { return m_position; }
		glm::vec3 GetSize() const { return m_size; }
	private:
		glm::vec3 m_position;
		glm::vec3 m_size;
		const int m_RESOLUTION{ 1024 };
		const float m_NEARPLANE{ 0.1f };
		const float m_FARPLANE{ 500.f };
		Texture m_cubemap;
	};
}