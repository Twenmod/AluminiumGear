#include "RealEngine.h"
#include "Inventory.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_pi.h"

#include "PlayerUI.h"

Inventory::Inventory(unsigned int _id, E_INPUTACTIONS _useAction, glm::vec2 _windowPosition) :
	m_WINDOWPOS(_windowPosition),
	m_ID(_id),
	m_USEACTION(_useAction)
{

}

void Inventory::Init(real::InputManager& _inputManager, PlayerUIManager& _uiManager, real::RealEngine& _engine)
{
	m_inputManager = &_inputManager;
	_inputManager.Subscribe(this);
	m_uiManager = &_uiManager;
	m_engine = &_engine;
}

Inventory::~Inventory()
{
	m_inputManager->UnSubscribe(this);
	m_items.clear();
}

void Inventory::DrawUI()
{
	static bool open = true;
	ImGui::SetNextWindowPos(ImVec2(m_WINDOWPOS.x, m_WINDOWPOS.y));
	ImGui::SetNextWindowSize(ImVec2(500, 500));
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_Draw3D | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground;
	ImGui::Begin("InventoryHud " + m_ID, &open, window_flags);
	ImGui::SetWindowFontScale(1.6f);
	if (m_active)
	{
		ImGui::Text("<");
		ImGui::SameLine(87, 0); ImGui::Text(">");
	}
	else ImGui::NewLine();
	if (m_selectedItem < m_items.size() && m_items[m_selectedItem] != nullptr)
	{
		ImGui::NewLine();
		ImGui::Text(" ");
		ImGui::SameLine(); ImGui::Text(m_items[m_selectedItem]->GetName().c_str());
		if (m_items[m_selectedItem].get()->GetHasAmount())
		{
			ImGui::SameLine(); ImGui::Text((std::to_string(m_items[m_selectedItem]->GetAmount()) + "/" + std::to_string(m_items[m_selectedItem]->GetMaxAmount())).c_str());
		}
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(m_WINDOWPOS.x + 20, m_WINDOWPOS.y));
	ImGui::SetNextWindowSize(ImVec2(50, 50));
	ImGui::Begin("InventoryHudSlot" + m_ID, &open, window_flags);
	ImGui::Image(0, ImVec2(50, 50), ImVec2(0, 0), ImVec2(0, 0), ImVec4(0.4f, 0.4f, 1.f, 0.5f), ImVec4(0.4f, 0.4f, 1.f, 0.5f));
	ImGui::SameLine(); ImGui::Image(0, ImVec2(50, 50), ImVec2(0, 0), ImVec2(0, 0), ImVec4(0.4f, 0.4f, 1.f, 0.5f), ImVec4(0.4f, 0.4f, 1.f, 0.5f));
	ImGui::End();
}
void Inventory::Draw3D(real::Camera& _mainCamera)
{

	//Draw item inside the box
	if (m_selectedItem < m_items.size() && m_items[m_selectedItem] != nullptr)
	{
		float offset = (std::max(m_animationTimer / m_ANIMATIONTIME, 0.f));
		float size = 1.f - offset;
		offset *= 50;
		if (m_switchDirectionIsRight) offset *= -1;
		glm::mat4 itemModel = m_uiManager->GetModel();
		itemModel = glm::translate(itemModel, glm::vec3(m_WINDOWPOS.x + 55 + offset, m_WINDOWPOS.y + 35, 0));
		itemModel = glm::scale(itemModel, glm::vec3(size));
		m_items[m_selectedItem]->Draw(_mainCamera.GetViewProjection(), itemModel);
	}
	//Animation
	if (m_animationTimer > 0)
	{
		int itemOffset = m_switchDirectionIsRight ? 1 : -1;
		int item = static_cast<int>(fmod(m_selectedItem + itemOffset, m_items.size()));
		if (item < m_items.size() && m_items[item] != nullptr)
		{
			float offset = (1.0f - std::max(m_animationTimer / m_ANIMATIONTIME, 0.f));
			float size = 1.f - offset;
			offset *= 50;
			offset *= itemOffset;
			glm::mat4 itemModel = m_uiManager->GetModel();
			itemModel = glm::translate(itemModel, glm::vec3(m_WINDOWPOS.x + 55 + offset, m_WINDOWPOS.y + 35, 0));
			itemModel = glm::scale(itemModel, glm::vec3(size));

			m_items[item]->Draw(_mainCamera.GetViewProjection(), itemModel);
		}
	}
}

void Inventory::Tick(float _deltaTime)
{
	if (m_selectedItem < m_items.size() && m_items[m_selectedItem] != nullptr)
		m_items[m_selectedItem]->Tick(_deltaTime);
	m_animationTimer -= _deltaTime;
	if (m_animationTimer <= 0) m_animationTimer = 0;
}

void Inventory::SetActive(bool _active)
{
	m_active = _active;
}

void Inventory::OnInputAction(real::InputAction _action)
{
	if (m_uiManager->m_mainMenu) return;

	//Not inside switch since its not constant
	if (_action.action == m_USEACTION)
	{
		if (m_selectedItem < m_items.size() && m_items[m_selectedItem] != nullptr && (!m_items[m_selectedItem].get()->GetHasAmount() || m_items[m_selectedItem].get()->GetAmount() > 0))
			m_items[m_selectedItem]->Use(_action.value);
		return;
	}
	if (m_active)
	{
		switch (_action.action)
		{
			case E_INPUTACTIONS::UI_HORIZONTAL:
				if (_action.value == 1)
				{
					m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/equip.wav");

					m_selectedItem = static_cast<unsigned int>(fmod(m_selectedItem + 1, m_items.size()));
					m_switchDirectionIsRight = true;
					m_animationTimer = m_ANIMATIONTIME;
				}
				else if (_action.value == -1)
				{
					m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/equip.wav");

					m_selectedItem = static_cast<unsigned int>(fmod(m_selectedItem - 1, m_items.size()));
					m_switchDirectionIsRight = false;
					m_animationTimer = m_ANIMATIONTIME;
				}
				break;
		}
	}
}

void Inventory::AddItem(std::unique_ptr<Item> _itemToAdd)
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (std::strcmp(m_items[i].get()->GetName().c_str(), _itemToAdd.get()->GetName().c_str()) == 0)
		{
			m_items[i].get()->Add(_itemToAdd.get()->GetAmount());
			return;
		}
	}
	_itemToAdd.get()->SetInventory(*this);
	m_items.push_back(std::move(_itemToAdd));
	
}

void Inventory::DeleteItem(const char* itemName)
{
	for (int i = 0; i < m_items.size(); i++)
	{
		if (std::strcmp(m_items[i].get()->GetName().c_str(), itemName) == 0)
		{
			m_items.erase(m_items.begin() + i);
		}
	}
}
