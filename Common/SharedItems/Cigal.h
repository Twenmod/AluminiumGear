#pragma once

#include "Item.h"

class Player;

class Cigal : public Item
{
public:
	Cigal(const Item& baseItem);
	~Cigal() {
	};
	void Init(real::Model* itemModel, Player& player, real::RealEngine& engine);
	void Use(float actionValue) override;
private:
	Player* m_player{ nullptr };
	real::RealEngine* m_engine;
};