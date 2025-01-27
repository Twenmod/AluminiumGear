#pragma once
class Item;
class Inventory;

class GroundItem : public real::GameObject, real::ICollisionCallback
{
public:
	GroundItem();
	~GroundItem();
	void Init(real::Model& model, Inventory& inventoryToAddTo, real::ObjectManager& objectManager, std::unique_ptr<Item> _item, real::RealEngine& engine);
	void Tick(float deltaTime) override;
private:
	void OnCollision(real::GameObject& collidingObject) override;
	std::unique_ptr<Item> m_item;
	Inventory* m_inventory{ nullptr };
	real::ObjectManager* m_objectManager;
	const float m_rotateSpeed{ 45.f };
	real::RealEngine* m_engine{ nullptr };

};