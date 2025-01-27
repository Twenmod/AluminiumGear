#include "RealEngine.h"
#include "PlayerUI.h"
#include "ImGui/imgui.h"
#include "Player.h"

#include "Cigal.h"
#include "GrenadeItem.h"

PlayerUIManager::PlayerUIManager()
{

}

PlayerUIManager::~PlayerUIManager()
{
	delete m_weaponInventory;
	delete m_equipmentInventory;
}

void PlayerUIManager::Init(real::InputManager& _inputManager, real::RealEngine& _engine, Player* _player)
{
	real::UIManager::Init(_engine, _player);
	m_player = _player;
	if (m_inputManager != nullptr) m_inputManager->UnSubscribe(this);
	m_inputManager = &_inputManager;
	_inputManager.Subscribe(this);
	if (m_weaponInventory != nullptr)  delete m_weaponInventory;
	if (m_equipmentInventory != nullptr)  delete m_equipmentInventory;
	m_weaponInventory = new Inventory(0, E_INPUTACTIONS::WEAPON_USE, glm::vec2(72.f, 120.f));
	m_equipmentInventory = new Inventory(1, E_INPUTACTIONS::ITEM_USE, glm::vec2(72.f, 200.f));
	m_weaponInventory->Init(_inputManager, *this, *m_engine);
	m_weaponInventory->SetActive(true);
	m_equipmentInventory->Init(_inputManager, *this, *m_engine);

	Item newItem = Item("Fists", 5.0f,false);
	newItem.Init(&_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Fist.fbx", *_engine.m_shaderManager.GetShader("Hologram")));
	m_weaponInventory->AddItem(std::move(std::make_unique<Item>(newItem)));

	Cigal newItem2 = Cigal(Item("Cigal", 7.0f, true, 20,20,true));
	newItem2.Init(&_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Cigal.fbx", *_engine.m_shaderManager.GetShader("Hologram")), *_player, _engine);
	m_equipmentInventory->AddItem(std::move(std::make_unique<Cigal>(newItem2)));


}


void PlayerUIManager::Tick(float _deltaTime)
{
	m_weaponInventory->Tick(_deltaTime);
	m_equipmentInventory->Tick(_deltaTime);
}

void PlayerUIManager::Draw(real::Camera& _mainCamera, float& _averageFPS, bool& _drawWireFrame,bool& _drawBulletDebug, bool& quitLevel, bool& quitDesktop, bool& startGame, bool& _fullscreen)
{
	//Pause menu
	ImGuiWindowFlags window_flags;

	if (m_mainMenu)
	{
		ImGui::SetNextWindowBgAlpha(0.8f);
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(0.f, static_cast<float>(m_windowSize.y/2-150)));
		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_windowSize.x), 400.f));
		ImGui::Begin("Main Menu", NULL, window_flags);
		ImGui::SetWindowFontScale(3.0f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + m_windowSize.x/2-170);
		ImGui::Text("  Aluminium Gear");
		ImGui::SetWindowFontScale(1.0f);
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + m_windowSize.x / 2 - 100);
		ImGui::Text("by: Jack Tollenaar");
		ImGui::NewLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + m_windowSize.x / 2-100);
		bool play = ImGui::Button("Play", ImVec2(200, 50));
		if (play)
		{
			m_engine->m_audioManager.PlayMusic("../Common/Assets/Audio/gameTheme.wav", MUSIC_VOLUME);
			m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
			m_paused = false;
			m_inputManager->GetMouse().ChangeMouseLockMode(real::MouseLockModes::LOCKEDANDHIDDEN);
			startGame = true;
		}
		ImGui::NewLine();

		ImGui::NewLine();
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + m_windowSize.x / 2-100);
		bool quit = ImGui::Button("Exit To Desktop", ImVec2(200, 50));
		if (quit)
		{
			m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/exit.wav");
			quitDesktop = true;
		}

		ImGui::End();

		ImGui::SetNextWindowBgAlpha(0.8f);
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x - 250), static_cast<float>(50)));
		ImGui::SetNextWindowSize(ImVec2(400, 400));
		ImGui::Begin("Logo", NULL, window_flags);
		ImGui::Image(m_engine->m_textureManager.GetTexture("../Common/Assets/buas.png",real::E_BASE_TEXTURE_TYPES::TEXTURE_TYPE_DIFFUSE).id, ImVec2(150, 150));
		ImGui::End();


		ImGui::SetNextWindowBgAlpha(0.8f);
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x-200), static_cast<float>(m_windowSize.y - 150)));
		ImGui::SetNextWindowSize(ImVec2(400, 400));
		ImGui::Begin("Settings", NULL, window_flags);
		ImGui::SetWindowFontScale(1.7f);
		ImGui::Checkbox("Fullscreen",&_fullscreen);
		ImGui::Checkbox("Debug menu", &m_drawDebug);
		ImGui::End();

		ImGui::SetNextWindowBgAlpha(0.8f);
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(0), static_cast<float>(m_windowSize.y - 240)));
		ImGui::SetNextWindowSize(ImVec2(400, 400));
		ImGui::Begin("Control", NULL, window_flags);
		ImGui::SetWindowFontScale(2.0f);
		ImGui::Text("Controls");
		ImGui::SetWindowFontScale(1.5f);
		ImGui::Text("[ESC]   ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": menu");
		ImGui::Text("[WASD]  ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0,1,0.5,1),": movement");
		ImGui::Text("[SHIFT] ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": run (hold)");
		ImGui::Text("[ARROWS]");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": HUD");
		ImGui::Text("[SPACE] ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": use equiped item");
		ImGui::Text("[RMB]   ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": Primary fire");
		ImGui::Text("[LMB]   ");
		ImGui::SameLine(); ImGui::TextColored(ImVec4(0, 1, 0.5, 1), ": Secondary fire (toggle)");
		ImGui::End();
	}

	if (!m_mainMenu && m_player != nullptr && m_player->GetHealth() <= 0 && !m_finishScreen)
	{
		ImGui::SetNextWindowBgAlpha(0.2f);
		window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x / 2 - 200), static_cast<float>(m_windowSize.y/2-150)));
		ImGui::Begin("DeathScreen", NULL, window_flags);
		ImGui::SetWindowFontScale(3.f);
		ImGui::SetCursorPosX(75);
		ImGui::Text("- GAME OVER -");
		ImGui::SetWindowFontScale(1.7f);
		ImGui::NewLine();
		bool returnMenu = ImGui::Button("Exit To Menu", ImVec2(200, 50));
		if (returnMenu)
		{
			m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
			quitLevel = true;
		}
		ImGui::SameLine();
		bool quit = ImGui::Button("Exit To Desktop", ImVec2(200, 50));
		if (quit)
		{
			m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/exit.wav");

			quitDesktop = true;
		}
		ImGui::End();
	}
	else
	{
		if (m_finishScreen)
		{
			ImGui::SetNextWindowBgAlpha(0.2f);
			window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x / 2 - 200), static_cast<float>(m_windowSize.y/2-150)));
			ImGui::Begin("FinishScreen", NULL, window_flags);
			ImGui::SetWindowFontScale(3.f);
			ImGui::SetCursorPosX(8);
			ImGui::Text("- MISSION SUCCESS -");
			ImGui::SetWindowFontScale(1.7f);
			ImGui::NewLine();
			bool returnMenu = ImGui::Button("Exit To Menu", ImVec2(200, 50));
			if (returnMenu)
			{
				m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
				m_finishScreen = false;
				quitLevel = true;
			}
			ImGui::SameLine();
			bool quit = ImGui::Button("Exit To Desktop", ImVec2(200, 50));
			if (quit)
			{
				m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/exit.wav");
				quitDesktop = true;
			}
			ImGui::End();
		}else if (m_paused && !m_mainMenu)
		{
			ImGui::SetNextWindowBgAlpha(0.2f);
			window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x / 2 - 100), static_cast<float>(m_windowSize.y/2-150)));
			ImGui::SetNextWindowSize(ImVec2(400.f, static_cast<float>(m_windowSize.y)));
			ImGui::Begin("Pause menu", NULL, window_flags);
			ImGui::SetWindowFontScale(1.6f);
			ImGui::Text("(not) Paused");
			ImGui::NewLine();
			bool resume = ImGui::Button("Resume", ImVec2(200, 50));
			if (resume)
			{
				m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
				m_paused = false;
				m_inputManager->GetMouse().ChangeMouseLockMode(real::MouseLockModes::LOCKEDANDHIDDEN);
			}
			ImGui::NewLine();
			ImGui::NewLine();
			bool returnMenu = ImGui::Button("Exit To Menu", ImVec2(200, 50));
			if (returnMenu)
			{
				m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
				quitLevel = true;
			}
			ImGui::NewLine();
			bool quit = ImGui::Button("Exit To Desktop", ImVec2(200, 50));
			if (quit)
			{
				m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/exit.wav");
				quitDesktop = true;
			}
			ImGui::End();

			ImGui::SetNextWindowBgAlpha(0.8f);
			window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x - 200), static_cast<float>(m_windowSize.y - 150)));
			ImGui::SetNextWindowSize(ImVec2(400.f, 400.f));
			ImGui::Begin("Settings", NULL, window_flags);
			ImGui::SetWindowFontScale(1.7f);
			ImGui::Checkbox("Fullscreen", &_fullscreen);
			ImGui::Checkbox("Debug menu", &m_drawDebug);
			ImGui::End();
		}
	}
	if (!m_mainMenu && !(m_player != nullptr && m_player->GetHealth() <= 0))
	{
		static bool open = true;
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::SetNextWindowPos(ImVec2(100.f, 90.f));
		window_flags = ImGuiWindowFlags_Draw3D | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;
		ImGui::Begin("3d Hud ", &open, window_flags);
		ImGui::SetWindowFontScale(1.6f);
		ImGui::Text(("Health " + std::to_string(static_cast<int>(m_player->GetHealth()))).c_str());
		ImGui::End();


		m_weaponInventory->DrawUI();
		m_equipmentInventory->DrawUI();
	}
	if (!m_mainMenu && m_drawDebug) DrawDebug(_averageFPS, _drawWireFrame, m_engine->m_loadedScene->GetNavigationMesh()->debugDraw, _drawBulletDebug); // Also draw debug UI
	EndFrame();
	if (!m_mainMenu && !(m_player != nullptr && m_player->GetHealth() <= 0))
	{
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		m_weaponInventory->Draw3D(_mainCamera);
		m_equipmentInventory->Draw3D(_mainCamera);
		glCullFace(GL_BACK);
	}
}

void PlayerUIManager::OnInputAction(real::InputAction _action)
{
	if (m_player != nullptr && m_player->GetHealth() <= 0) return;

	switch (_action.action)
	{
		case E_INPUTACTIONS::UI_VERTICAL:
			if (m_mainMenu) break;
			if (_action.value == 1) m_inventorySelected = static_cast<unsigned int>(fmod(m_inventorySelected + 1, 2));
			else if (_action.value == -1) m_inventorySelected = static_cast<unsigned int>(fmod(m_inventorySelected - 1, 2));
			else
			{
				break;
			}
			m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/equip.wav");
			//Switch active inventory
			if (m_inventorySelected == 0)
			{
				m_weaponInventory->SetActive(true);
				m_equipmentInventory->SetActive(false);
			}
			else if (m_inventorySelected == 1)
			{
				m_weaponInventory->SetActive(false);
				m_equipmentInventory->SetActive(true);
			}

			break;
		case E_INPUTACTIONS::PAUSE:
			if (_action.value == 1)
			{
				m_paused = !m_paused;
				if (m_paused)
				{
					m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
					m_inputManager->GetMouse().ChangeMouseLockMode(real::MouseLockModes::UNLOCKED);
				}
				else
				{
					m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/select.wav");
					m_inputManager->GetMouse().ChangeMouseLockMode(real::MouseLockModes::LOCKEDANDHIDDEN);
				}
			}
			break;
	}
}
