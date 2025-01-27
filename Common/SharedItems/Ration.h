#pragma once

#include "Item.h"

class Player;

class Ration : public Item
{
public:
	Ration(const Item& baseItem);
	~Ration() {
	};
	void Init(real::Model* itemModel, Player& player, real::RealEngine& engine);
	void Use(float actionValue) override;
private:
	Player* m_player{ nullptr };
	real::RealEngine* m_engine;
};