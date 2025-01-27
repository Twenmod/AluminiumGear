#include "RealEngine.h"
#include "GrenadeItem.h"

#include "Player.h"
#include "Grenade.h"

GrenadeItem::GrenadeItem(const Item& baseItem) : Item(baseItem)
{
}

void GrenadeItem::Init(real::Model* _itemModel, Player& _player, real::RealEngine& _engine)
{
	Item::Init(_itemModel);
	m_player = &_player;
	m_engine = &_engine;
}

void GrenadeItem::Use(float actionValue)
{
	if (actionValue == 1)
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/throw.wav");

		Grenade* newGrenade = new Grenade(25.f, 50.f, 0.1f, 3.f);
		newGrenade->Init(*m_engine,
			m_engine->m_modelManager.GetModel("../Common/Assets/Models/Items/Grenade.obj", *m_engine->m_shaderManager.GetShader(BASE_SHADER)),
			real::E_GENERATECOLLISIONSHAPEMODE::GENERATE_HULL,
			1.6f, m_player->GetPosition() + glm::vec3(0.f, 1.f, 0.f));
		m_engine->m_loadedScene->GetObjectManager().AddObject(*newGrenade);
		m_engine->m_btDynamicsWorld->addRigidBody(newGrenade->m_body, BTGROUP_GRENADE, BTGROUP_ALL &~ (BTGROUP_PLAYER));
		newGrenade->m_body->applyCentralImpulse(GlmVecToBtVec(m_player->GetRotation() * glm::vec3(0, 0.3, 1)*30.f));

		Remove(1);
	}
	else if (actionValue == -1)
	{
	}
	else
	{

	}
}