#include "precomp.h"
#include "Game.h"

#include "ImGui/imgui.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/imgui_impl_pi.h"

#include "RealMath.h"

#include "IGraphics.h"

#include "IInputEvent.h"

#include "IInput.h"
#include "Input.h"

#include "TextureManager.h"
#include "Model.hpp"
#include "Shader.h"
#include "GameObject.h"
#include "Camera.h"
#include "Light.h"

#include "BodyMotionState.h"

#include "ReflectionProbe.h"
#include "NavMesh.h"
#include "ParticleSystem.h"
#include "ComputeShader.h"

//Game
#include "ThirdPersonCamera.hpp"
#include "Player.h"
#include "WaterModel.h"
#include "GibbedGameObject.h"

#include "Animation.h"
#include "Animator.h"

#include "Enemy.h"
#include "Inventory.h"
#include "GroundItem.h"
#include "Pistol.h"
#include "Ration.h"
#include "FlickeringLight.h"
#include "GrenadeItem.h"

#include "BTDebugDrawer.h"

Game::Game(real::InputManager* _input, IGraphics* _graphics) :
	m_input(_input),
	m_graphics(_graphics)
{
	m_windowSize = glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT);
}

Game::~Game()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplPi_Shutdown();
	ImGui::DestroyContext();

	if (m_player != nullptr) m_engine.m_btDynamicsWorld->removeRigidBody(m_player->m_body);
	delete m_player;
	delete m_customDebugDrawer;
	delete m_postprocessFrame;
}

float timer = 0;

void Game::Start()
{

	InitializeOpenGLES();

	//ImGui Setup
	const char* glsl_version = "#version 100";  //310/320es not available
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplOpenGL3_Init(glsl_version);
	ImGui_ImplPi_InitForOpenGL();
	ImGuiIO io = ImGui::GetIO();
	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
	io.DisplaySize = ImVec2(static_cast<float>(m_windowSize.x), static_cast<float>(m_windowSize.y));
	m_uiManager.SetWindowSize(m_windowSize);


	//Engine setup
	m_engine.Init();
	m_customDebugDrawer = m_engine.m_debugDrawer;


	m_uiManager.Init(*m_input, m_engine, m_player);

	//Load scene
	LoadScene(0, glm::vec3(0), false, false, true);

	//Set up framebuffer for post procesing
	glGenFramebuffers(1, &m_postProcessBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessBuffer);
	//Create a texture
	glGenTextures(1, &m_postProcessRenderTexture.id);
	glBindTexture(GL_TEXTURE_2D, m_postProcessRenderTexture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Bind the texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_postProcessRenderTexture.id, 0);
	//Set up render buffer for depth and stencil

	glGenTextures(1, &m_postProcessDepthTexture.id);
	glBindTexture(GL_TEXTURE_2D, m_postProcessDepthTexture.id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_postProcessDepthTexture.id, 0);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "Framebuffer is not complete! Status: " << status << std::endl;
	}

	m_postprocessFrame = new real::Model();
	m_postprocessFrame->Init(&m_engine.m_textureManager, real::Primitives::PRIMITIVE_PLANE, m_postProcessRenderTexture, *m_engine.m_shaderManager.GetShader("PostProcess"));

	// Timing
	auto startTime = std::chrono::system_clock::now();
	auto lastTime = startTime;

	while (!m_quitting)
	{
		//Calculate DeltaTime
		auto time = std::chrono::system_clock::now();
		std::chrono::duration<float> delta = time - lastTime;
		m_gameDeltaTime = std::min(delta.count(), 0.3f); // Clamp deltatime to avoid falling through world on lagspike
		std::chrono::duration<float> elapsed = time - startTime;
		if (elapsed.count() > 0.25f && m_frameCount > 10)
		{
			m_averageFPS = static_cast<float>(m_frameCount) / elapsed.count();
			startTime = time;
			m_frameCount = 0;
		}

		Update(m_gameDeltaTime);
		Render();
		PostRender();

		lastTime = time;
		++m_frameCount;

	}

	m_graphics->Quit();
}

void Game::Update(float _deltaTime)
{
	timer += _deltaTime;
	m_input->Tick();
	m_input->TickImGui();
	m_engine.m_loadedScene->Tick(_deltaTime);
	if (m_player != nullptr) m_player->Tick(_deltaTime);
	m_uiManager.Tick(_deltaTime);

	//Combat audio
	bool enemiesTriggered = false;
	for (real::GameObject* object : m_engine.m_loadedScene->GetObjectManager().GetObjects())
	{
		if (Enemy* enemy = dynamic_cast<Enemy*>(object))
		{
			if (enemy->GetIsTriggered()) enemiesTriggered = true;
		}
	}
	if (enemiesTriggered && !m_inCombat)
	{
		m_inCombat = true;
		m_engine.m_audioManager.PlayMusic("../Common/Assets/Audio/encounter.wav", MUSIC_VOLUME);
	}
	else if (!enemiesTriggered && m_inCombat)
	{
		m_inCombat = false;
		m_engine.m_audioManager.PlayMusic("../Common/Assets/Audio/gameTheme.wav", MUSIC_VOLUME);
	}

	//Levelloading test
	if (m_player != nullptr)
	{
		switch (m_engine.m_loadedScene->m_sceneID)
		{
			case 1:
				if (m_player->GetPosition().z < -130.0f) // Next scene
				{
					m_uiManager.DrawLoadingScreen(m_graphics, *m_input, false);
					LoadScene(2, glm::vec3(60.23, 0, -125.72f));
				}
				break;
			case 2:
				if (m_player->GetPosition().z > 10) // backwards
				{
					m_uiManager.DrawLoadingScreen(m_graphics, *m_input, false);
					LoadScene(1, glm::vec3(-60.23f, 0, 125.72f));
				}
				else if (m_player->GetPosition().z < -95) // forward
				{
					m_uiManager.DrawLoadingScreen(m_graphics, *m_input, false);
					LoadScene(3, glm::vec3(-59.575f, 0, -120.87f));
				}
				break;
			case 3:
				if (m_player->GetPosition().z > 35) // backwards
				{
					m_uiManager.DrawLoadingScreen(m_graphics, *m_input, false);
					LoadScene(2, glm::vec3(59.575f, 0, 120.87f));
				}
				if (m_player->GetPosition().z < -120)
				{
					if (!m_uiManager.m_finishScreen) m_engine.m_audioManager.PlaySoundFile("../Common/Assets/Audio/victory.wav");
					m_uiManager.m_finishScreen = true;
				}
				break;
		}

	}
	m_engine.Tick();
	m_engine.StepPhysics(m_gameDeltaTime);
}
void Game::Render()
{
	//Render to FB
	glBindFramebuffer(GL_FRAMEBUFFER, m_postProcessBuffer);
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(RENDER_WIDTH, RENDER_HEIGHT);
	// Setup the viewport
	ClearScreen();
	glViewport(0, 0, RENDER_WIDTH, RENDER_HEIGHT);

	if (m_player != nullptr)
	{
		glm::mat4 imGuiModel = glm::mat4(1);
		imGuiModel = glm::translate(imGuiModel, m_player->GetPosition() + glm::vec3(0, 3, 0));
		imGuiModel *= glm::mat4_cast(m_player->GetRotation());
		imGuiModel = glm::scale(imGuiModel, glm::vec3(-0.01f));
		m_uiManager.SetupMVP(m_engine.m_loadedScene->GetMainCamera().GetViewProjection(), imGuiModel);
	}
	if (m_player != nullptr)
	{
		m_player->SetupDraw(m_engine.m_loadedScene->GetMainCamera().GetViewProjection(), std::vector<real::ReflectionProbe*>(), m_drawWireFrame);
		m_player->GetModel()->Draw(std::vector<real::ReflectionProbe*>(), m_drawWireFrame);
	}
	m_engine.m_loadedScene->Draw(m_drawWireFrame);

	//Draw any water objects seperately
	for (real::GameObject* object : m_engine.m_loadedScene->GetObjectManager().GetObjects())
	{
		if (Water* water = dynamic_cast<Water*>(object))
		{
			water->SetupDraw(m_engine.m_loadedScene->GetMainCamera().GetViewProjection(), m_engine.m_loadedScene->GetObjectManager().GetSelectedReflectionProbes(), m_drawWireFrame);
			water->Draw();
		}
	}


	GLuint uiShader = ImGui_ImplOpenGL3_GetShader();
	if (uiShader != 0)
	{
		glUseProgram(uiShader);
		glUniform1f(glGetUniformLocation(uiShader, "time"), timer);
	}

	if (m_drawBulletDebug)
		m_engine.PhysicsDebugDraw();

	if (m_engine.m_loadedScene->GetNavigationMesh() != nullptr) m_engine.m_loadedScene->GetNavigationMesh()->Draw();

	m_customDebugDrawer->SetViewProjection(m_engine.m_loadedScene->GetMainCamera().GetViewProjection());
	m_customDebugDrawer->Draw();


	//Render FB to main buffer
	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
	io.DisplaySize = ImVec2(static_cast<float>(m_windowSize.x), static_cast<float>(m_windowSize.y));
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	ClearScreen();
	const real::Shader& postProcessShader = m_postprocessFrame->GetShader();
	postProcessShader.use();
	postProcessShader.setMat4("Model", glm::mat4(1));
	postProcessShader.setMat4("VP", glm::mat4(1));

	m_postprocessFrame->Draw(std::vector<real::ReflectionProbe*>(), false);

	//Render imgui overlays

	m_uiManager.StartFrame();
	bool startGameFromMenu = false;
	bool gotoMenu = false;
	bool oldfullscreen = m_fullScreen;
	m_uiManager.Draw(m_engine.m_loadedScene->GetMainCamera(), m_averageFPS, m_drawWireFrame, m_drawBulletDebug, gotoMenu, m_quitting, startGameFromMenu, m_fullScreen);
	if (m_fullScreen != oldfullscreen) SetFullScreen(m_fullScreen);
	if (startGameFromMenu)
	{
		LoadScene(1, glm::vec3(0), false, true, true, true);
	}
	if (gotoMenu)
	{
		LoadScene(0, glm::vec3(0), false, false, true);
	}

	glFlush();
	m_graphics->SwapBuffer();

}

void Game::PostRender()
{

}

void Game::LoadScene(int _sceneID, glm::vec3 _startLoadPoint, bool _movePlayer, bool _spawnPlayer, bool _recreatePlayer, bool _clearScreen)
{

	if (_sceneID != 0)
	{
		m_uiManager.m_mainMenu = false;
		m_input->GetMouse().ChangeMouseLockMode(real::MouseLockModes::LOCKEDANDHIDDEN);
		m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.f);
	}

	delete m_engine.m_loadedScene;
	m_engine.m_loadedScene = new real::Scene();

	m_engine.m_loadedScene->Init(m_engine.m_shaderManager, *m_engine.m_btDynamicsWorld, *m_customDebugDrawer);

	m_engine.m_loadedScene->SetMainCamera(m_player->m_thirdPersonCam);

	m_engine.m_loadedScene->m_sceneID = _sceneID;

	if (_recreatePlayer && m_player != nullptr)
	{
		m_engine.m_btDynamicsWorld->removeRigidBody(m_player->m_body);
		delete m_player;
		m_player = nullptr;

	}

	if ((_spawnPlayer && m_player == nullptr))
	{
		m_player = new Player();

		m_player->m_name = "Player";
		m_player->Init(*m_input, m_engine.m_modelManager.GetModel("../Common/Assets/Models/Player/ModelIdle.dae", *m_engine.m_shaderManager.GetShader("Skeletal")), m_engine);
		m_player->SetPosition(glm::vec3(0, 5.f, 0));
		m_player->SetScale(glm::vec3(0.135f, 0.135f, 0.135f));
		m_player->m_body->setActivationState(DISABLE_DEACTIVATION);
		m_engine.m_btDynamicsWorld->addRigidBody(m_player->m_body, BTGROUP_PLAYER, BTGROUP_ALL);
		m_engine.m_loadedScene->SetMainCamera(m_player->m_thirdPersonCam);
		m_engine.m_audioManager.SetListener(m_player->m_thirdPersonCam);
		m_uiManager.Init(*m_input, m_engine, m_player);
	}


	//Spawn objects
	switch (_sceneID)
	{
		case 0:
		{
			m_engine.m_audioManager.PlayMusic("../Common/Assets/Audio/mainTheme.wav", MUSIC_VOLUME);
			real::Camera* menuCam = new real::Camera();
			menuCam->Init(m_input);
			m_engine.m_loadedScene->SetMainCamera(*menuCam);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*menuCam);
			m_uiManager.m_mainMenu = true;
			m_input->GetMouse().ChangeMouseLockMode(real::MouseLockModes::UNLOCKED);
			if (m_uiManager.m_equipmentInventory != nullptr) m_uiManager.m_equipmentInventory->SetActive(false);
			if (m_uiManager.m_equipmentInventory != nullptr) m_uiManager.m_weaponInventory->SetActive(false);
			break;
		}
		case 1:
		{

#pragma region lights

			const float baseLightFallofLinear = 0.007f;
			const float baseLightFallofQuadratic = 0.015f;
			FlickeringLight* pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Corridor Light L";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(-13, 12, -23));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Corridor Light R";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(14.5, 12, -23));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Corridor Light2 L";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(-13, 12, -43));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Corridor Light2 R";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(14.5, 12, -43));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Side Light L";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(-44, 12, -26));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Side Light R";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(63, 12, -10));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight =new FlickeringLight(0.2f,glm::vec2(0.4f,20.f),glm::vec2(0.1f,0.7f),real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Point Light";
			pointLight->Init(m_engine);
			pointLight->SetPosition(glm::vec3(0.8, 10, -57));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			real::Light* normalPointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f, 0, 0), glm::vec3(.2f), 0.8f, baseLightFallofLinear, baseLightFallofQuadratic);
			normalPointLight->m_name = "Thruster Light";
			normalPointLight->Init();
			normalPointLight->SetPosition(glm::vec3(-18, 5, -70));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*normalPointLight);

			normalPointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f, 0, 0), glm::vec3(.2f), 0.8f, baseLightFallofLinear, baseLightFallofQuadratic);
			normalPointLight->m_name = "Thruster Light";
			normalPointLight->Init();
			normalPointLight->SetPosition(glm::vec3(-18, 5, -85));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*normalPointLight);

			normalPointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f, 0, 0), glm::vec3(.2f), 0.8f, baseLightFallofLinear, baseLightFallofQuadratic);
			normalPointLight->m_name = "Thruster Light";
			normalPointLight->Init();
			normalPointLight->SetPosition(glm::vec3(-18, 5, -105));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*normalPointLight);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.1f);

#pragma endregion
			real::Model& level = m_engine.m_modelManager.GetModel("../Common/Assets/Maps/map1/level1.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER));
			real::GameObject* mapObject = new real::GameObject();
			mapObject->m_name = "MapObject";
			mapObject->Init(level, real::E_GENERATECOLLISIONSHAPEMODE::GENERATE_PRECISE);
			//mapObject->SetPosition(glm::vec3(15, 0, 15));
			//mapObject->SetScale(glm::vec3(0.2f));
			mapObject->m_body->setCollisionFlags(mapObject->m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
			m_engine.m_btDynamicsWorld->addRigidBody(mapObject->m_body,BTGROUP_DEFAULT,BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*mapObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.3f);

			//NavMesh
			real::NavMesh& navMesh = *new real::NavMesh();
			navMesh.Init(m_customDebugDrawer);
			navMesh.load("../Common/Assets/Maps/map1/level1Nav.obj");
			m_engine.m_loadedScene->SetNavigationMesh(navMesh);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.4f);

			//Generate reflectProbe before spawning dynamic geometry
			real::ReflectionProbe* probe = new real::ReflectionProbe(glm::vec3(0, 5.5, 0), glm::vec3(125, 20, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(63, 3.f, -10), glm::vec3(60, 15, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);


			probe = new real::ReflectionProbe(glm::vec3(-50, 5.5, -25), glm::vec3(40, 20, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(0.4, 5.5, -30), glm::vec3(34, 20, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(0, 5.5, -55), glm::vec3(20, 15, 20), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-8, 15, -90), glm::vec3(44, 45, 60), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(34, 0, -94), glm::vec3(50, 15, 15), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(61, 0, -120), glm::vec3(25, 15, 50), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.6f);

			Water* water = new Water(glm::vec2(10.f, 50.f));
			water->Init(*m_engine.m_shaderManager.GetShader("Water"), m_engine.m_perlin);
			water->SetPosition(glm::vec3(-18.f, 0, -88.f));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*water);

			water = new Water(glm::vec2(50.f, 50.f));
			water->Init(*m_engine.m_shaderManager.GetShader("Water"), m_engine.m_perlin);
			water->SetPosition(glm::vec3(64.f, -1.3f, -10.f));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*water);

			real::Model& enemyModel = m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/WalkModel.dae", *m_engine.m_shaderManager.GetShader("Skeletal"));
			Enemy* enemyObject = new Enemy();
			enemyObject->m_name = "Enemy";
			enemyObject->Init(navMesh, m_engine, enemyModel, "../Common/Assets/Models/Enemy/Idle.dae", m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/Skeleton.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), 80, glm::vec3(-8, 6, -70), glm::vec3(2.2, 2.2, 2.2));
			enemyObject->SetTarget(*m_player);
			enemyObject->SetScale(glm::vec3(0.025f));
			std::vector<glm::vec3> enemyPath = {
				glm::vec3(-8, 2, -70),
				glm::vec3(-8, 2, -95),
				glm::vec3(-22,2, -95),
				glm::vec3(-8, 2, -95),
				glm::vec3(-8, 2, -110)
			};
			enemyObject->SetPatrolPath(enemyPath);
			m_engine.m_btDynamicsWorld->addRigidBody(enemyObject->m_body, BTGROUP_ENEMY,BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*enemyObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.84f);

			Pistol newItem = Pistol(Item("Pistol", 15.0f,true,5,10,false));
			newItem.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Pistol.obj", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player,m_engine.m_audioManager);
			GroundItem* groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Pistol.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_weaponInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<Pistol>(newItem), m_engine);
			groundItem->SetPosition(glm::vec3(-44, 2, -26));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);

			Ration newItem2 = Ration(Item("Ration", 24.0f, true, 1, 10, true));
			newItem2.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Ration.fbx", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine);
			groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Ration.fbx", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<Ration>(newItem2), m_engine);
			groundItem->SetPosition(glm::vec3(-11, 4, -43));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);

			groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Ration.fbx", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<Ration>(newItem2), m_engine);
			groundItem->SetPosition(glm::vec3(83, 1, -10));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.92f);

			GrenadeItem newItem3 = GrenadeItem(Item("Grenade", 40.0f, true, 3, 10, true));
			newItem3.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine);

			groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<GrenadeItem>(newItem3), m_engine);
			groundItem->SetPosition(glm::vec3(-20, 2, -115));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);


			real::GibbedObjectsSpawner* boxes = new real::GibbedObjectsSpawner();
			boxes->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Boxes.obj", *m_engine.m_shaderManager.GetShader("NoReflec")), m_engine.m_textureManager, 3.f, glm::vec3(-20, 2, -114));
			boxes->AddToDynamicsWorld(*m_engine.m_btDynamicsWorld);
			boxes->AddToScene(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddGibObjects(*boxes);

			//real::Texture& skyBoxTexture = engine.textureManager.GetTexture(skyBoxFaces, real::TEXTURE_TYPE_CUBEMAP); 

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 1.0f);

			break;
		}
		case 2:
		{
			real::Model& level = m_engine.m_modelManager.GetModel("../Common/Assets/Maps/map1/level2.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER));
			real::GameObject* mapObject = new real::GameObject();
			mapObject->m_name = "MapObject";
			mapObject->Init(level, real::E_GENERATECOLLISIONSHAPEMODE::GENERATE_PRECISE);
			//mapObject->SetPosition(glm::vec3(0, 15, 0));
			//mapObject->SetScale(glm::vec3(0.2f));
			mapObject->m_body->setCollisionFlags(mapObject->m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
			m_engine.m_btDynamicsWorld->addRigidBody(mapObject->m_body, BTGROUP_DEFAULT, BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*mapObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.3f);
			//NavMesh
			real::NavMesh& navMesh = *new real::NavMesh();
			navMesh.Init(m_customDebugDrawer);
			navMesh.load("../Common/Assets/Maps/map1/level2Nav.obj");
			m_engine.m_loadedScene->SetNavigationMesh(navMesh);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.4f);

			const float baseLightFallofLinear = 0.007f;
			const float baseLightFallofQuadratic = 0.015f;
			real::Light* pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Aparatus light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-38.5, 0, -27));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Aparatus light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-80, 0, -27));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);


			FlickeringLight* flickeringLight = new FlickeringLight(0.2f, glm::vec2(0.4f, 20.f), glm::vec2(0.1f, 0.7f), real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			flickeringLight->m_name = "stair light L";
			flickeringLight->Init(m_engine);
			flickeringLight->SetPosition(glm::vec3(-85, -6, -51));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*flickeringLight);

			flickeringLight = new FlickeringLight(0.2f, glm::vec2(0.4f, 20.f), glm::vec2(0.1f, 0.7f), real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			flickeringLight->m_name = "stair light L";
			flickeringLight->Init(m_engine);
			flickeringLight->SetPosition(glm::vec3(-75, -6, -51));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*flickeringLight);

			flickeringLight = new FlickeringLight(0.2f, glm::vec2(0.4f, 20.f), glm::vec2(0.1f, 0.7f), real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			flickeringLight->m_name = "stair light R";
			flickeringLight->Init(m_engine);
			flickeringLight->SetPosition(glm::vec3(-43, -6, -51));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*flickeringLight);

			flickeringLight = new FlickeringLight(0.2f, glm::vec2(0.4f, 20.f), glm::vec2(0.1f, 0.7f), real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			flickeringLight->m_name = "stair light R";
			flickeringLight->Init(m_engine);
			flickeringLight->SetPosition(glm::vec3(-33, -6, -51));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*flickeringLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Wall lights";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-43, -16, -78));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear - 0.002f, baseLightFallofQuadratic);
			pointLight->m_name = "Wall lights";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-75, -16, -78));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 4, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-60, -25, -123));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 2, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-40, -17, -123));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 2, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-80, -17, -123));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.5f);

			real::ReflectionProbe* probe = new real::ReflectionProbe(glm::vec3(0.77, 0, 5.72), glm::vec3(25, 15, 50), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-3, -3, -30), glm::vec3(30, 15, 25), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-60, -1.5f, -35), glm::vec3(90, 40, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-74, -16.f, -75), glm::vec3(30, 20, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-44, -16, -75), glm::vec3(30, 20, 40), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(-60, -10.f, -123), glm::vec3(55, 80, 55), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.6f);


			Pistol newItem = Pistol(Item("Pistol", 15.0f, true, 5, 10, false));
			newItem.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Pistol.obj", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine.m_audioManager);
			GroundItem* groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Pistol.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_weaponInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<Pistol>(newItem), m_engine);
			groundItem->SetPosition(glm::vec3(-80, -16, -61));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);

			Ration newItem2 = Ration(Item("Ration", 24.0f, true, 1, 10, true));
			newItem2.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Ration.fbx", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine);
			groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Ration.fbx", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<Ration>(newItem2), m_engine);
			groundItem->SetPosition(glm::vec3(-38, -16, -61));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.7f);


			real::Model& enemyModel = m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/WalkModel.dae", *m_engine.m_shaderManager.GetShader("Skeletal"));
			Enemy* enemyObject = new Enemy();
			enemyObject->m_name = "Enemy";
			enemyObject->Init(navMesh, m_engine, enemyModel, "../Common/Assets/Models/Enemy/Idle.dae", m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/Skeleton.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), 80, glm::vec3(-40, -5, -35), glm::vec3(2.2, 2.2, 2.2));
			enemyObject->SetTarget(*m_player);
			enemyObject->SetScale(glm::vec3(0.025f));
			std::vector<glm::vec3> enemyPath = {
				glm::vec3(-40, -6, -35),
				glm::vec3(-60, -8, -35),
				glm::vec3(-60, -8, -48),
				glm::vec3(-60, -8, -24),
				glm::vec3(-60, -8, -35),
				glm::vec3(-95, -6, -35)
			};
			enemyObject->SetPatrolPath(enemyPath);
			m_engine.m_btDynamicsWorld->addRigidBody(enemyObject->m_body, BTGROUP_ENEMY, BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*enemyObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.83f);


			GrenadeItem newItem3 = GrenadeItem(Item("Grenade", 40.0f, true, 2, 10, true));
			newItem3.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine);

			groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<GrenadeItem>(newItem3), m_engine);
			groundItem->SetPosition(glm::vec3(-22, -2, -24));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);


			real::GibbedObjectsSpawner* boxes = new real::GibbedObjectsSpawner();
			boxes->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Boxes.obj", *m_engine.m_shaderManager.GetShader("NoReflec")), m_engine.m_textureManager, 3.f, glm::vec3(-22, -6, -45));
			boxes->AddToDynamicsWorld(*m_engine.m_btDynamicsWorld);
			boxes->AddToScene(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddGibObjects(*boxes);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.95f);
			Water* water = new Water(glm::vec2(25.f, 25.f));
			water->Init(*m_engine.m_shaderManager.GetShader("Water"), m_engine.m_perlin);
			water->SetPosition(glm::vec3(-60.f, -22, -123.f));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*water);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 1.f);
			break;
		}
		case 3:
		{
			real::Model& level = m_engine.m_modelManager.GetModel("../Common/Assets/Maps/map1/level3.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER));
			real::GameObject* mapObject = new real::GameObject();
			mapObject->m_name = "MapObject";
			mapObject->Init(level, real::E_GENERATECOLLISIONSHAPEMODE::GENERATE_PRECISE);
			//mapObject->SetPosition(glm::vec3(0, 15, 0));
			//mapObject->SetScale(glm::vec3(0.2f));
			mapObject->m_body->setCollisionFlags(mapObject->m_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
			m_engine.m_btDynamicsWorld->addRigidBody(mapObject->m_body, BTGROUP_DEFAULT, BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*mapObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.2f);

			//NavMesh
			real::NavMesh& navMesh = *new real::NavMesh();
			navMesh.Init(m_customDebugDrawer);
			navMesh.load("../Common/Assets/Maps/map1/level3Nav.obj");
			m_engine.m_loadedScene->SetNavigationMesh(navMesh);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.3f);

			const float baseLightFallofLinear = 0.007f;
			const float baseLightFallofQuadratic = 0.015f;
			real::Light* pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear, baseLightFallofQuadratic);
			pointLight->m_name = "Wall lights";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-43, -16, -78) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear - 0.002f, baseLightFallofQuadratic);
			pointLight->m_name = "Wall lights";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-75, -16, -78) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 4, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-60, -25, -123) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 2, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-40, -17, -123) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(0.4f, 0.4f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 2, baseLightFallofQuadratic / 2);
			pointLight->m_name = "Reactor light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(-80, -17, -123) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			pointLight = new real::Light(real::E_lightTypes::pointLight, glm::vec3(1.f, 1.f, 1.f), glm::vec3(.2f), 1.f, baseLightFallofLinear / 3, baseLightFallofQuadratic / 3);
			pointLight->m_name = "Room Light";
			pointLight->Init();
			pointLight->SetPosition(glm::vec3(0, 0, -46));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*pointLight);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.4f);

			real::ReflectionProbe* probe = new real::ReflectionProbe(glm::vec3(-60, -10.f, -123)+glm::vec3(59.575,0,120.87), glm::vec3(55, 80, 55), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(0, -14, -46), glm::vec3(30, 36, 30), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			probe = new real::ReflectionProbe(glm::vec3(0, -7, -90), glm::vec3(60, 40, 60), REFLECTION_RESOLUTION);
			probe->Probe(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddReflectionProbe(*probe);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.64f);
			real::Model& enemyModel = m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/WalkModel.dae", *m_engine.m_shaderManager.GetShader("Skeletal"));
			Enemy* enemyObject = new Enemy();
			enemyObject->m_name = "Enemy";
			enemyObject->Init(navMesh, m_engine, enemyModel, "../Common/Assets/Models/Enemy/Idle.dae", m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/Skeleton.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), 80, glm::vec3(0, -17, -16), glm::vec3(2.2, 2.2, 2.2));
			enemyObject->SetTarget(*m_player);
			enemyObject->SetScale(glm::vec3(0.025f));
			std::vector<glm::vec3> enemyPath = {
				glm::vec3(0, -17, -16),
				glm::vec3(0, -17, -37),
				glm::vec3(3, -17, -51),
				glm::vec3(16, -17, -91),
				glm::vec3(0, -17, -107),
				glm::vec3(-16, -17, -91),
				glm::vec3(-3, -17, -51),
				glm::vec3(0, -17, -37)
			};
			enemyObject->SetPatrolPath(enemyPath);
			m_engine.m_btDynamicsWorld->addRigidBody(enemyObject->m_body, BTGROUP_ENEMY, BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*enemyObject);


			enemyObject = new Enemy();
			enemyObject->m_name = "Enemy";
			enemyObject->Init(navMesh, m_engine, enemyModel, "../Common/Assets/Models/Enemy/Idle.dae", m_engine.m_modelManager.GetModel("../Common/Assets/Models/Enemy/Skeleton.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), 80, glm::vec3(0, -17, -107), glm::vec3(2.2, 2.2, 2.2));
			enemyObject->SetTarget(*m_player);
			enemyObject->SetScale(glm::vec3(0.025f));
			enemyPath = {
				glm::vec3(0, -17, -107),
				glm::vec3(16, -17, -91),
				glm::vec3(3, -17, -51),
				glm::vec3(0, -17, -37),
				glm::vec3(0, -17, -16),
				glm::vec3(0, -17, -37),
				glm::vec3(-3, -17, -51),
				glm::vec3(-16, -17, -91)
			};
			enemyObject->SetPatrolPath(enemyPath);
			m_engine.m_btDynamicsWorld->addRigidBody(enemyObject->m_body, BTGROUP_ENEMY, BTGROUP_ALL);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*enemyObject);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.78f);

			GrenadeItem newItem3 = GrenadeItem(Item("Grenade", 40.0f, true, 8, 10, true));
			newItem3.Init(&m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader("Hologram")), *m_player, m_engine);

			GroundItem* groundItem = new GroundItem();
			groundItem->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine.m_shaderManager.GetShader(BASE_SHADER)), *m_uiManager.m_equipmentInventory, m_engine.m_loadedScene->GetObjectManager(), std::make_unique<GrenadeItem>(newItem3), m_engine);
			groundItem->SetPosition(glm::vec3(-4, -17, -112));
			groundItem->SetRotation(glm::quat(glm::vec3(glm::radians(180.f), 0, 0)));
			m_engine.m_btDynamicsWorld->addRigidBody(groundItem->m_body);
			m_engine.m_loadedScene->GetObjectManager().AddObject(*groundItem);


			real::GibbedObjectsSpawner* boxes = new real::GibbedObjectsSpawner();
			boxes->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Boxes.obj", *m_engine.m_shaderManager.GetShader("NoReflec")), m_engine.m_textureManager, 3.f, glm::vec3(0, -17, -90));
			boxes->AddToDynamicsWorld(*m_engine.m_btDynamicsWorld);
			boxes->AddToScene(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddGibObjects(*boxes);

			real::GibbedObjectsSpawner* boxes2 = new real::GibbedObjectsSpawner();
			boxes2->Init(m_engine.m_modelManager.GetModel("../Common/Assets/Models/Boxes.obj", *m_engine.m_shaderManager.GetShader("NoReflec")), m_engine.m_textureManager, 3.f, glm::vec3(0, -16, -90));
			boxes2->AddToDynamicsWorld(*m_engine.m_btDynamicsWorld);
			boxes2->AddToScene(m_engine.m_loadedScene->GetObjectManager());
			m_engine.m_loadedScene->GetObjectManager().AddGibObjects(*boxes2);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 0.93f);

			Water* water = new Water(glm::vec2(25.f, 25.f));
			water->Init(*m_engine.m_shaderManager.GetShader("Water"), m_engine.m_perlin);
			water->SetPosition(glm::vec3(-60.f, -22, -123.f) + glm::vec3(59.575, 0, 120.87));
			m_engine.m_loadedScene->GetObjectManager().AddObject(*water);

			m_uiManager.DrawLoadingScreen(m_graphics, *m_input, _clearScreen, 1.f);
			break;
		}
	}

	if (_movePlayer && m_player != nullptr)
	{
		glm::vec3 offset = m_player->GetPosition() - _startLoadPoint;
		m_player->SetPosition(offset); // Set player to world origin with an offset from the scene
		m_player->m_thirdPersonCam.Tick(0); // Make sure camera also follows
	}



}

void Game::FinishLoading()
{
	m_uiManager.m_loading = false;
	//TODO: hide loading screen
}

const real::InputManager& Game::GetInput() const
{
	return *m_input;
}

void Game::Quit()
{
	m_quitting = true;
}

void Game::ResizeWindow(glm::ivec2 newSize)
{
	m_windowSize = newSize;
	m_uiManager.SetWindowSize(newSize);
}

void Game::SetFullScreen(bool _fullScreen)
{
	m_fullScreen = _fullScreen;
	m_graphics->SetFullScreen(_fullScreen);
}

void Game::InitializeOpenGLES()
{

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LEQUAL);
	glDepthRangef(0.0f, 1.0f);
	glClearDepthf(1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glViewport(0, 0, m_windowSize.x, m_windowSize.y);
}

void Game::ClearScreen()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Dont clear color since that isnt needed in a sealed level
}
