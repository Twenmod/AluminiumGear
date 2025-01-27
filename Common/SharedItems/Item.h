#pragma once

class Inventory;

class Item
{
public:
	Item(std::string name = "Unnamed Item", float modelScale = 1.0f, bool hasAmount = true, unsigned int amount = 1, unsigned int maxAmount = 10, bool consumable = true);
	virtual ~Item();
	virtual void Init(real::Model* itemModel);
	void Draw(const glm::mat4& viewProjection, const glm::mat4& modelMatrix);
	void Tick(float deltaTime);
	//Called when use inputaction is called
	virtual void Use(float) {};
	const std::string& GetName() const { return m_NAME; }
	unsigned int GetAmount() const { return m_amount; }
	bool GetHasAmount() const { return m_HASAMOUNT; }
	void Add(unsigned int amount);
	void Remove(unsigned int amount);
	unsigned int GetMaxAmount() const { return m_MAXAMOUNT; }
	void SetInventory(Inventory& inventory);
protected:
	Inventory* m_inventory;
	real::Model* m_itemModel;
	const float m_SPINANIMATIONSPEED{ 45.f };
	float m_rotation{ 0 };

	const std::string m_NAME{ "Unnamed Item" };
	const bool m_HASAMOUNT{ true };
	unsigned int m_amount{ 1 };
	const unsigned int m_MAXAMOUNT{ 10 };
	const bool m_CONSUMABLE{ true }; // Dissapear after amount == 0
	const float m_MODELSCALE{ 1.0f };

};