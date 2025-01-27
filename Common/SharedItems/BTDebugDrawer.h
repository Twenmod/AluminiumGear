#pragma once
#include <btBulletDynamicsCommon.h>
#include "Light.h"

namespace real
{
	class ShaderManager;
	class Camera;

	class BtDebugDrawer : public btIDebugDraw
	{
	public:
		int m_debugMode;

		BtDebugDrawer();

		void Init(ShaderManager& shaderManager);

		void SetViewProjection(glm::mat4 _vp) { m_vp = _vp; }

		void Draw();

		virtual void setDebugMode(int _debugMode) override
		{
			m_debugMode = _debugMode;
		}

		virtual int getDebugMode() const override
		{
			return m_debugMode;
		}

		virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;

		virtual void drawContactPoint(const btVector3& _pointOnB, const btVector3& _normalOnB,
			btScalar _distance, int, const btVector3& _color) override
		{
			btVector3 to = _pointOnB + _normalOnB * _distance;
			drawLine(_pointOnB, to, _color);
		}

		void drawLightProbe(btVector3 _pos, btVector3 _size, btVector3 _color)
		{
			if (!drawProbes) return;
			drawSphere(_pos, 0.5f, _color);
			drawBox(_pos - _size / 2.f, _pos + _size / 2.f,_color);
		}
		void drawLight(Light& _light)
		{
			if (!drawLights) return;
			drawSphere(GlmVecToBtVec(_light.GetPosition()), 0.2f, btVector3(0, 1, 1));
			if (_light.GetType() != E_lightTypes::pointLight)
			{
				drawLine(GlmVecToBtVec(_light.GetPosition()), GlmVecToBtVec(_light.GetPosition() + _light.GetDirection() * 2.f),btVector3(0.f,0.7f,1.f));
			}
		}

		virtual void reportErrorWarning(const char* _warningString) override
		{
			printf("Bullet Debug Warning: %s\n", _warningString);
		}

		virtual void draw3dText(const btVector3&, const char*) override
		{
		}
		bool drawProbes = false;
		bool drawLights = false;

	private:
		ShaderManager* m_shaderManager{ nullptr };
		GLuint m_vbo;
		Camera* m_camera{ nullptr };
		glm::mat4 m_vp;
		std::vector<GLfloat> m_lineData;
	};
}