#pragma once
#include "precomp.h"

namespace real
{
	class GameObject;

	class ICollisionCallback
	{
	public:
		ICollisionCallback(btCollisionObject& _body) : m_collisionBody(_body) {}
		const btCollisionObject& GetBody() const { return m_collisionBody; }
		virtual void OnCollision(GameObject& collidingObject) = 0;
	protected:
		btCollisionObject& m_collisionBody;
	};
}