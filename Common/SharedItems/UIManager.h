#pragma once

	class IGraphics;

namespace real
{
	class RealEngine;
	class GameObject;
	class InputManager;

	class UIManager
	{
	public:
		UIManager();
		virtual void Init(RealEngine& engine, GameObject* player = nullptr);
		void StartFrame(bool updateInput = true);
		void EndFrame(bool updateInput = true);
		virtual void Draw() {};
		void DrawDebug(const float averageFPS, bool& drawWireFrame, bool& navMeshDraw, bool& bulletDebugDraw);
		void SetupMVP(glm::mat4 viewProjection,glm::mat4 model);
		glm::mat4 GetModel() const { return m_model; }
		void DrawLoadingScreen(IGraphics* _graphics, real::InputManager& inputManager, bool clearScreen = true, float progress = -1);
		void SetWindowSize(glm::ivec2 _windowSize) { m_windowSize = _windowSize; };
	protected:
		RealEngine* m_engine;
		//This is only for the DebugScreen
		GameObject* m_playerObject;
		glm::mat4 m_model;
		glm::mat4 m_viewProjection;
		glm::ivec2 m_windowSize{ glm::ivec2(WINDOW_WIDTH,WINDOW_HEIGHT) };
	};
}