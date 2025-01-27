#include "RealEngine.h"
#include "Item.h"
#include "Inventory.h"

Item::Item(std::string _name, float _modelScale, bool _hasAmount, unsigned int _amount, unsigned int _maxAmount, bool _consumable) :
	m_NAME(_name),
	m_HASAMOUNT(_hasAmount),
	m_MAXAMOUNT(_maxAmount),
	m_CONSUMABLE(_consumable),
	m_MODELSCALE(_modelScale)
{
	m_amount = _amount;
}
Item::~Item()
{
}

void Item::Init(real::Model* _itemModel)
{
	m_itemModel = _itemModel;
}

void Item::Draw(const glm::mat4& _viewProjection, const glm::mat4& _modelMatrix)
{
	//bool wasDepthOn = glIsEnabled(GL_DEPTH);
	//glDisable(GL_DEPTH);

	//Set correct position etc
	const real::Shader& itemShader = m_itemModel->GetShader();
	itemShader.use();

	glm::mat4 model = glm::rotate(_modelMatrix, glm::radians(m_rotation), glm::vec3(0, 1, 0));
	model = glm::scale(model, glm::vec3(m_MODELSCALE));
	itemShader.setMat4("Model", model);
	itemShader.setMat4("VP", _viewProjection);

	m_itemModel->Draw(std::vector<real::ReflectionProbe*>(), false);

	//glEnable(GL_DEPTH);

}

void Item::Tick(float deltaTime)
{
	m_rotation = fmod(m_rotation + deltaTime * m_SPINANIMATIONSPEED, 360.f);
}

void Item::Add(unsigned int _amount)
{
	m_amount = std::min(m_amount + _amount,m_MAXAMOUNT);
}

void Item::Remove(unsigned int _amount)
{
	m_amount -= _amount;
	if (m_amount <= 0)
	{
		if (m_CONSUMABLE)
		{
			m_inventory->DeleteItem(m_NAME.c_str());
		}
		else m_amount = 0;
	}
}

void Item::SetInventory(Inventory& _inventory)
{
	m_inventory = &_inventory;
}
