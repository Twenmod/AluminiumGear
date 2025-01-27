#include "precomp.h"
#include "UIManager.h"

#include "RealEngine.h"

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_opengl3.h"

#include "IGraphics.h"

real::UIManager::UIManager()
{

}

void real::UIManager::Init(real::RealEngine& _engine, GameObject* _player)
{
	m_engine = &_engine;
	m_playerObject = _player;
}

void real::UIManager::StartFrame(bool _updateInput)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame(_updateInput);
}

void real::UIManager::EndFrame(bool _updateInput)
{
	ImGui_ImplOpenGL3_SetupMVP((m_viewProjection * m_model));
	ImGui::EndFrame(_updateInput);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void real::UIManager::DrawDebug(const float _averageFPS, bool& _drawWireFrame, bool& _navMeshDraw, bool& _bulletDebugDraw)
{
	//Default debug drawings

	ImGui::SetNextWindowBgAlpha(0.2f);
	ImGuiWindowFlags window_flags = /*ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar  | */ ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;
	static bool open = true;
	ImGui::Begin("Debug ", &open, window_flags);
	ImGui::SetWindowFontScale(1.6f);

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	bool treeOpen = ImGui::TreeNodeEx("Runtime Info");
	if (treeOpen)
	{
		ImGui::Text("FPS: %.f (%.2f ms)", _averageFPS, 1 / _averageFPS * 1000);
		ImGui::TreePop();
	}
	treeOpen = ImGui::TreeNodeEx("Rendering");
	if (treeOpen)
	{
		ImGui::Checkbox("Debug Camera", &m_engine->m_loadedScene->GetMainCamera().m_debugMode);
		ImGui::Checkbox("Wireframe", &_drawWireFrame);
		ImGui::Text("Debug Draw options");
		ImGui::Checkbox("Lights", &m_engine->m_debugDrawer->drawLights);
		ImGui::Checkbox("ReflectionProbes", &m_engine->m_debugDrawer->drawProbes);
		ImGui::Checkbox("NavMesh", &_navMeshDraw);
		ImGui::Checkbox("Draw colliders", &_bulletDebugDraw);
		ImGui::TreePop();
	}

	ImGui::End();

	//Hierarchy
	ImGui::SetNextWindowBgAlpha(0.2f);
	window_flags = /*ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar  | */ ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar;
	ImGui::Begin("Hierarchy ", &open, window_flags);
	ImGui::SetWindowFontScale(1.6f);
	bool rootOpen = ImGui::TreeNodeEx("Scene");
	if (rootOpen)
	{
		if (m_playerObject != nullptr)
		{
			std::string s = m_playerObject->m_name;
			s += "##";
			s += std::to_string(0);
			bool nodeOpen = ImGui::TreeNode(s.c_str());
			if (nodeOpen)
			{
				m_playerObject->DrawImGuiProperties();
				ImGui::TreePop();
			}
		}

		std::array<GameObject*, MAX_OBJECTS> objects = m_engine->m_loadedScene->GetObjectManager().GetObjects();
		int i = 0;
		for (GameObject* object : objects)
		{
			if (object != nullptr)
			{
				std::string s = object->m_name;
				s += "##";
				s += std::to_string(i);
				bool nodeOpen = ImGui::TreeNode(s.c_str());
				if (nodeOpen)
				{
					object->DrawImGuiProperties();
					ImGui::TreePop();
				}
			}
			i++;
		}
		ImGui::TreePop();
	}

	ImGui::End();
}

void real::UIManager::SetupMVP(glm::mat4 _viewProjection, glm::mat4 _model)
{
	m_viewProjection = _viewProjection;
	m_model = _model;
}

void real::UIManager::DrawLoadingScreen(IGraphics* _graphics, real::InputManager& _inputManager, bool _clearScreen, float _progress)
{
	
	if (ImGui::GetFrameActive())
	{
		EndFrame();
	}

	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	if (_clearScreen)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	ImGuiIO io = ImGui::GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(m_windowSize.x), static_cast<float>(m_windowSize.y));

	StartFrame();

	ImGui::SetNextWindowBgAlpha(1.0f);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs;
	ImGui::SetNextWindowPos(ImVec2(static_cast<float>(m_windowSize.x / 2 - 60), static_cast<float>(m_windowSize.y / 2 - 20)));
	if (_progress >= 0) ImGui::SetNextWindowSize(ImVec2(120, 80));
	else ImGui::SetNextWindowSize(ImVec2(120, 40));
	ImGui::Begin("loading screen", NULL, window_flags);
	ImGui::SetWindowFontScale(2.f);
	ImGui::Text("Loading");
	if (_progress >= 0)
	{
		ImGui::SetCursorPosX(45);
		ImGui::Text((std::to_string(static_cast<int>(_progress * 100.f))).c_str());
	}
		ImGui::End();

	EndFrame();
	glFlush();
	_graphics->SwapBuffer();

	_inputManager.IgnoreNextFrameMouseMovement();


}
