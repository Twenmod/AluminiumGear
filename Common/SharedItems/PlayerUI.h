#pragma once
#include "Inventory.h"

class Player;

class PlayerUIManager : public real::UIManager, real::IInputEvent
{
public:
	PlayerUIManager();
	~PlayerUIManager();
	void Init(real::InputManager& inputManager, real::RealEngine& engine, Player* player);
	void Tick(float deltaTime);
	void Draw(real::Camera& mainCamera, float& _averageFPS, bool& _drawWireFrame, bool& _drawBulletDebug, bool& quitLevel, bool& quitDekstop, bool& startGame, bool& fullscreen);
	void OnInputAction(real::InputAction action);
	Inventory* m_weaponInventory;
	Inventory* m_equipmentInventory;
	bool m_loading{ false };
	bool m_mainMenu{ false };
	bool m_finishScreen{ false };
private:
	unsigned int m_inventorySelected{ 0 };
	bool m_drawDebug{ false };
	bool m_paused{ false };
	real::InputManager* m_inputManager{ nullptr };
	Player* m_player;
};