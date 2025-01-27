#pragma once

#include "Item.h"

class PlayerUIManager;

class Inventory : real::IInputEvent
{
public:
	Inventory(unsigned int id, E_INPUTACTIONS useAction, glm::vec2 windowPosition = glm::vec2(72, 120));
	~Inventory();
	void Init(real::InputManager& inputManager, PlayerUIManager& uiManager, real::RealEngine& engine);
	//Draw the ImGui parts of the inventory call first
	void DrawUI();
	//Draws the 3d parts of the inventory call after calling ImGuiImpl Render
	void Draw3D(real::Camera& mainCamera);
	void Tick(float deltaTime);
	void SetActive(bool active);
	void OnInputAction(real::InputAction action);
	void AddItem(std::unique_ptr<Item> itemToAdd);
	void DeleteItem(const char* itemName);

private:
	std::vector<std::unique_ptr<Item>> m_items;
	bool m_active{ false };
	unsigned int m_selectedItem{ 0 };
	const glm::vec2 m_WINDOWPOS;
	real::RealEngine* m_engine{nullptr};
	real::InputManager* m_inputManager{ nullptr };
	PlayerUIManager* m_uiManager{ nullptr };
	const float m_ANIMATIONTIME{ 0.2f };
	const unsigned int m_ID{ 0 };
	float m_animationTimer;
	bool m_switchDirectionIsRight;
	const E_INPUTACTIONS m_USEACTION{ E_INPUTACTIONS::NONE };
};