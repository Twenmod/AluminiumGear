#pragma once





//Quality settings
#ifdef WINDOWS_BUILD
constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 800;
constexpr unsigned int RENDER_WIDTH = 640;
constexpr unsigned int RENDER_HEIGHT = 360;

constexpr float ASPECT_RATIO = 1280.f / 800.f;
#define REFLECTIONS true
constexpr unsigned int REFLECTION_RESOLUTION = 1024;
constexpr int PHYSICS_TICKRATE = 60;
constexpr float LIGHT_CULL_MIN = 30.f * 30.f; // dont cull lights closer than sqrt(this)
constexpr float LIGHT_CULL_DOT_SOFT = 0.f; // Cull light if dot product < this
constexpr float LIGHT_CULL_DOT_HARD = -0.5f;
constexpr float LIGHT_CULL_MAX = 200.f * 200.f; // Cull lights further than sqrt(this)
constexpr char BASE_SHADER[] = "BaseShader";
#else // Pi build
constexpr unsigned int WINDOW_WIDTH = 800;
constexpr unsigned int WINDOW_HEIGHT = 450;
constexpr unsigned int RENDER_WIDTH = 640;
constexpr unsigned int RENDER_HEIGHT = 360;
constexpr float ASPECT_RATIO = 800.0f / 450.0f;
#define REFLECTIONS true
constexpr unsigned int REFLECTION_RESOLUTION = 48;
constexpr int PHYSICS_TICKRATE = 30;
constexpr float LIGHT_CULL_MIN = 10.f * 10.f; // dont cull lights closer than sqrt(this)
constexpr float LIGHT_CULL_DOT_SOFT = 0.4f; // Cull light if dot product < this
constexpr float LIGHT_CULL_DOT_HARD = 0.25f;
constexpr float LIGHT_CULL_MAX = 60.f * 60.f; // Cull lights further than sqrt(this)
constexpr char BASE_SHADER[] = "SimpleShader";
#endif


//Audio
constexpr float MUSIC_VOLUME = 0.15f;

//Input

enum E_INPUTACTIONS
{
	VERTICAL_MOVE,
	HORIZONTAL_MOVE,
	SPRINT,
	JUMP_CROUCH,
	WEAPON_USE,
	ITEM_USE,
	INTERACT,
	PAUSE,
	UI_HORIZONTAL,
	UI_VERTICAL,
	NONE,
};
constexpr unsigned int INPUT_ACTION_AMOUNT = 10;

//Shaders

enum class E_MATERIALTEXTURETYPES
{
	MATERIAL_TEXTURE_TYPE_DIFFUSE,
	MATERIAL_TEXTURE_TYPE_SPECULAR,
	MATERIAL_TEXTURE_TYPE_EMISSIVE,
};

constexpr unsigned int MAX_MATERIAL_TEXTURES = 10;

constexpr char SHADER_FOLDER[] = "../Common/Assets/Shaders/";

constexpr int CUBEMAPTEXTUREPOSITION = 4; // 4 since 0-3 are taken by the material
constexpr int DATATEXTUREPOSITION = 6; // 6 since cubemaps take 2


///Animations
constexpr unsigned int MAX_BONES = 65;
constexpr unsigned int MAX_BONE_INFLUENCE = 60;
constexpr unsigned int MAX_BONE_WEIGHTS = 60;


//Scenes/Level

constexpr unsigned int MAX_OBJECTS = 1000;


constexpr unsigned int ROOM_TILE_WIDTH = 32;
constexpr unsigned int ROOM_TILE_HEIGTH = 24;


//Physics


enum E_BTPHYSICSGROUPS
{
	BTGROUP_DEFAULT = 1 << 1,
	BTGROUP_PLAYER = 1 << 2,
	BTGROUP_ENEMY = 1 << 3,
	BTGROUP_GIBS = 1 << 4,
	BTGROUP_GRENADE = 1 << 5,
	BTGROUP_NOCOLLIDE = 1 << 15,
	BTGROUP_ALL = 0b1111111111111111 & ~BTGROUP_NOCOLLIDE, //all 15 groups
};