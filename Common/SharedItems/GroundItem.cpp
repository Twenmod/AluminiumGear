#include "RealEngine.h"
#include "GroundItem.h"
#include "Inventory.h"

GroundItem::GroundItem() : real::GameObject(), ICollisionCallback(*m_body)
{
	btCollisionShape* m_shape = new btSphereShape(1.f);

	m_body->setCollisionShape(m_shape);
	m_body->setRestitution(0.1f);
	m_body->setFriction(1.f);
	m_body->setAngularFactor(btVector3(0, 0, 0)); // Lock rotation
}

GroundItem::~GroundItem()
{
	m_engine->UnSubscribeToCollisionCallback(*this);
	delete m_body->getCollisionShape();
}

void GroundItem::Init(real::Model& _model, Inventory& _inventoryToAddTo, real::ObjectManager& _objectManager, std::unique_ptr<Item> _item, real::RealEngine& _engine)
{
	m_inventory = &_inventoryToAddTo;
	m_objectManager = &_objectManager;
	m_item = std::move(_item);
	m_engine = &_engine;
	m_engine->SubscribeToCollisionCallback(*this);
	GameObject::Init(_model, real::E_GENERATECOLLISIONSHAPEMODE::DONT_GENERATE, 10.f);
}

void GroundItem::Tick(float _deltaTime)
{
	SetRotation(glm::rotate(GetRotation(), glm::radians(m_rotateSpeed * _deltaTime), glm::vec3(0, 1, 0)));
	GameObject::Tick(_deltaTime);
}

void GroundItem::OnCollision(real::GameObject& _collidingObject)
{
	//Check if it is the player
	if (_collidingObject.m_name == "Player")
	{
		m_engine->m_audioManager.PlaySoundFile("../Common/Assets/Audio/pickup.wav",1.f,true,GetPosition(),0.f);
		//Add itself to the inventory
		m_inventory->AddItem(std::move(m_item));
		m_objectManager->RemoveObject(*this);
	}
}
