#include "RealEngine.h"
#include "Pistol.h"

#include "Player.h"

Pistol::Pistol(const Item& baseItem) : Item(baseItem)
{
}

void Pistol::Init(real::Model* _itemModel, Player& _player, real::AudioManager& _audioManager)
{
	Item::Init(_itemModel);
	m_player = &_player;
	m_audioManager = &_audioManager;
}

void Pistol::Use(float actionValue)
{
	if (actionValue == 1 && m_player->GetIsAiming())
	{
		m_player->Shoot();
		m_audioManager->PlaySoundFile("../Common/Assets/Audio/pew.wav");
		if (m_amount == 1) m_player->Aim(); // Un Aim if last bullet
		Remove(1);
	}else if (actionValue == -1)
	{
		m_player->Aim();
	}
	else
	{
		
	}
}