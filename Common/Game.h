#pragma once
#include "RealEngine.h"
#include "TextureManager.h"
#include "ObjectManager.h"
#include "ShaderManager.h"
#include "ModelManager.h"
#include "Scene.h"
#include "UIManager.h"
#include "PlayerUI.h"

struct ImFont;

namespace real
{

	class GameObject;
	class InputManager;
	class BtDebugDrawer;
}

class IGraphics;
class Player;
class Inventory;

class Game
{
public:
	Game(real::InputManager* input, IGraphics* graphics);
	virtual ~Game();
	void Start();
	const real::InputManager& GetInput() const;
	void Quit();
	void ResizeWindow(int newSizeX, int newSizeY) { ResizeWindow(glm::ivec2(newSizeX, newSizeY)); };
	void ResizeWindow(glm::ivec2 newSize);
	void SetFullScreen(bool fullScreen);
protected:
	void ProcessInput();
	virtual void Update(float /*gameDeltaTime*/);
	virtual void Render();
	virtual void PostRender();
	void LoadScene(int sceneID, glm::vec3 startLoadPoin = glm::vec3(0), bool movePlayer = true, bool spawnPlayer = true, bool deletePlayer = false, bool _clearScreen = false);
	//Call after adding everything to scene to stop the loading screen
	void FinishLoading();

	real::InputManager* m_input{ nullptr };
	bool m_quitting{ false };
	float m_gameDeltaTime;

	IGraphics* m_graphics;

private:
	void InitializeOpenGLES();
	void ClearScreen();
	GLuint m_postProcessBuffer;
	real::Texture m_postProcessRenderTexture;
	real::Texture m_postProcessDepthTexture;
	real::Model* m_postprocessFrame;

	glm::ivec2 m_windowSize;

	bool m_inCombat{ false };
	int m_frameCount{ 0 };
	float m_averageFPS{ 0 };
	bool m_loading{ false };
	real::RealEngine m_engine;
	PlayerUIManager m_uiManager;
	Player* m_player{ nullptr };
	real::BtDebugDrawer* m_customDebugDrawer{ nullptr };
	bool m_drawBulletDebug{ false };
	bool m_drawWireFrame{ false };
	bool m_fullScreen{ false };
};

