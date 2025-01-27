#pragma once

#include "Item.h"

class Player;

class Pistol : public Item
{
public:
	Pistol(const Item& baseItem);
	~Pistol() {
	};
	void Init(real::Model* itemModel, Player& player, real::AudioManager& audioManager);
	void Use(float actionValue) override;
private:
	Player* m_player{ nullptr };
	real::AudioManager* m_audioManager;
};