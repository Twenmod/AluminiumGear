#pragma once

#include "Item.h"

class Player;

class GrenadeItem : public Item
{
public:
	GrenadeItem(const Item& baseItem);
	~GrenadeItem()
	{
	};
	void Init(real::Model* itemModel, Player& player, real::RealEngine& engine);
	void Use(float actionValue) override;
private:
	Player* m_player{ nullptr };
	real::RealEngine* m_engine;
};